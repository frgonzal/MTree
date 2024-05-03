#include <cstdlib>
#include <cmath>
#include <vector>
#include <queue>
#include "../../headers/mtree.hpp"
#include "../../headers/point.hpp"
#include "../../headers/utils/random.hpp"


int variable_estatica = 0;

#include <stdio.h>
static void printf_vector(const std::vector<Point> &points){
    printf("[");
    for(int i=0; i<points.size(); i++){
        printf("(%.2f, %.2f)",points[i].x, points[i].y);
        if(i < points.size()-1)
            printf(", ");
    }
    printf("]\n");
}
static void printf_mtree(MTree *raiz){
    printf("\nMTree:\n");
    std::queue<MTree*> q;
    std::vector<std::vector<MTree*>> niveles(raiz->h+1);
    q.push(raiz);

    while(!q.empty()){
        MTree* mtree = q.front();
        q.pop();
        niveles[mtree->h].push_back(mtree);

        for(int i=0; i<mtree->a.size(); i++){
            q.push(&mtree->a[i]);
        }
    }

    for(int i=niveles.size()-1; i>=0; i--){
        printf("\t-- Nivel %d --\n", i);
        for(int j=0; j<niveles[i].size(); j++){

            MTree *mtree = niveles[i][j];
            printf("p:(%.2f, %.2f),h:%d,n:%ld,cr:%1.2f;\n",mtree->p.x, mtree->p.y, mtree->h, mtree->a.size(), mtree->cr);
        }
    }
    printf("\n");
}

static void assign_to_nearest(std::vector<Point> &f, std::vector<std::vector<Point>> &F, const std::vector<Point> &points);

static void redistribution(std::vector<Point> &f, std::vector<std::vector<Point>> &F);

static void update_radius(MTree *mtree, int h);

static void append_to_leaf(MTree *mtree, MTree child, Point p);

static std::vector<MTree> find_mtree_h(MTree *mtree, int h);


/** The bulk_loading algorithm */
MTree bulk_loading(const std::vector<Point> &points){
    int ticket = variable_estatica++;

    // Si |P| ≤ B, se crea un árbol T , se insertan todos los puntos a T y se retorna T
    if(points.size() <= B){
        MTree mtree = MTree(points[0]);
        for(int i=0; i<points.size(); i++)
            mtree.add_child(MTree(points[i]));
        return mtree;
    }


    std::vector<Point> f;
    std::vector<std::vector<Point>> F;

    int k;
    for(;;){
        k = B;

        // De manera aleatoria se eligen k = min(B, n/B) puntos de P, que los llamaremos samples pf1 , . . . , pfk . 
        // Se insertan en un conjunto F de samples.
        f = random_k_sample(points, k);

        // Se le asigna a cada punto en P su sample más cercano. Con eso se puede construir k conjuntos
        // F1, . . . , Fk 
        assign_to_nearest(f, F, points);

        // Etapa de redistribución: Si algún Fj es tal que |Fj| < b:
        redistribution(f, F);

        k = f.size();

        if(k > 1)
            break;

        f.clear();
        F.clear();
    }

    std::vector<MTree> T;
    std::vector<Point> f2;
    // Se realiza recursivamente el algoritmo CP en cada Fj, obteniendo el árbol T
    for (int i=0; i<f.size(); i++){
        MTree mtree = bulk_loading(F[i]);

        if(mtree.a.size() >= (B >> 1)){
            T.push_back(mtree);
            f2.push_back(f[i]);
        }else{ 
            // Si la raíz del árbol es de un tamaño menor a b
            // Se quita esa raíz se elimina pfj de F.
            // Se trabaja con sus subárboles como nuevos 
            // Tj , . . . , Tj+p−1, se añaden los puntos pertinentes a F.
            for(int j=0; j<mtree.a.size(); j++){
                T.push_back(mtree.a[j]);
                f2.push_back(mtree.a[j].p);
            }
        }
    }
    f.clear();
    f.shrink_to_fit();
    f = f2;


    // Etapa de balanceamiento: Se define h como la altura mínima de los árboles Tj. 
    int min_h = 1e8;

    // buscar altura minima entre los hijos
    for(int i=0; i<T.size(); i++){
        min_h = T[i].h < min_h ? T[i].h : min_h;
    }
    
    if(ticket == 0){
        printf_vector(f);
    }
    // Se define T' inicialmente como un conjunto vacío.
    std::vector<MTree> T_prima(0);
    f2 = std::vector<Point>(0);

    //Por cada Tj , si su altura es igual a h, se añade a T'
    for(int i=0; i<T.size(); i++){
        if(T[i].h == min_h){
            T_prima.push_back(T[i]);
            f2.push_back(f[i]);
        }else{ //Si no se cumple:
            //Se borra el punto pertinente en F.
            //Se hace una búsqueda exhaustiva en Tj de todos los subárboles T'_1, . . . ,T'_p 
            // de altura igual a h. 
            
            std::vector<MTree> mtree_altura_h = find_mtree_h(&T[i], min_h);
            // Se insertan estos árboles a T'
            // Se insertan los puntos raíz de T'_1, . . .,T'_p, f1, . . . ,fp en F
            for(int j=0; j<mtree_altura_h.size(); j++){
                T_prima.push_back(mtree_altura_h[j]);
                f2.push_back(mtree_altura_h[j].p);
            }
        }
    }
    f.clear();
    f.shrink_to_fit();
    f = f2;

    if(ticket == 0){
        for(int i=0; i<T_prima.size(); i++)
            printf_mtree(&T_prima[i]);
        printf_vector(f);
    }

    // Se define T_sup == mtree como el resultado de la llamada al algoritmo CP aplicado a F
    MTree mtree = bulk_loading(f);
    int h = mtree.h;

    //Se une cada Tj ∈ T' a su hoja en T_sup correspondiente al punto pfj ∈ F, 
    // obteniendo un nuevo árbol T .
    for(int i=0; i<T_prima.size(); i++){
        append_to_leaf(&mtree, T_prima[i], f[i]);
    }

    //Se setean los radios cobertores resultantes para cada entrada en este árbol.
    update_radius(&mtree, h);

    return mtree;
}


MTree* mtree_create_cp(const std::vector<Point> &points){
    MTree *mtree = new MTree();
    *mtree = bulk_loading(points);
    return mtree;
}

/** Assigns points to their neares sample */
static void assign_to_nearest(std::vector<Point> &f, std::vector<std::vector<Point>> &F, const std::vector<Point> &points){
    F.resize(f.size(), std::vector<Point>(0));
    for(int i=0; i<points.size(); i++){
        int k = -1;
        double min_d2 = 1e5;
        for(int j=0; j<f.size(); j++){
            double d2 = dist2(points[i], f[j]);
            if(d2 < min_d2){
                min_d2 = d2;
                k = j;
            }
        }
        F[k].push_back(points[i]);
    }
}


/** Recorre el sample en busca de los F_k que tq |F_k| < b */
static void redistribution(std::vector<Point> &f, std::vector<std::vector<Point>> &F){
    int i=0;
    while(i < f.size()){
        if(F[i].size() < (B >> 1)){
            std::vector<Point> points = F[i];
            f.erase(f.begin()+i);            
            F.erase(F.begin()+i);            
            assign_to_nearest(f, F, points);
        }else
            i++;
    }
}

/** Updated radius and height of an mtree */
static void update_radius(MTree *mtree, int h){
    if(mtree->h <= 0)
        return;

    for(int i=0; i<mtree->a.size(); i++)
        update_radius(&mtree->a[i], h-1);


    double max_d2 = 0;
    int max_h = 0;
    for(int i=0; i<mtree->a.size(); i++){
        double d2 = dist2(mtree->p, mtree->a[i].p);
        max_d2 = d2 > max_d2 ? d2 : max_d2;
        max_h = mtree->a[i].h > max_h ? mtree->a[i].h : max_h;
    }
    mtree->h = max_h + 1;
    mtree->cr = sqrt(max_d2);
}

/** Returns a vector with all the sub Trees that have height == h*/
static std::vector<MTree> find_mtree_h(MTree *mtree, int h){
    std::queue<MTree*> q;
    std::vector<MTree> mtrees_h;
    q.push(mtree);

    while(!q.empty()){
        MTree* mtree = q.front();
        q.pop();

        if(mtree->h == h){
            mtrees_h.push_back(*mtree);
            continue;
        }

        for(int i=0; i<mtree->a.size(); i++){
            q.push(&mtree->a[i]);
        }
    }

    return mtrees_h;
}

/** BFS para encontra la hoja */
static void append_to_leaf(MTree *root, MTree child, Point p){
    std::queue<MTree*> q;
    q.push(root);

    while(!q.empty()){
        MTree *mtree = q.front();
        q.pop();

        if(mtree->a.size() == 0 && mtree->h == 0 && point_equal(mtree->p, p)){
            *mtree = child;
            return;
        }
        for(int i=0; i<mtree->a.size(); i++){
            q.push(&mtree->a[i]);
        }
    }
}