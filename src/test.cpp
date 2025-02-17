#include <cstdlib>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <queue>
#include "../headers/point.hpp"
#include "../headers/mtree.hpp"
#include "../headers/utils/random.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <chrono>


static void printf_mtree(MTree *mtree, int power, std::string method){
    std::ostringstream fileName;
    fileName << "./resultados/" << method << "/mtree/result/" << power << ".csv";
    std::ofstream outFile(fileName.str());

    std::queue<Node*> q;
    q.push(mtree->root);

    while(!q.empty()){
        Node *node = q.front();
        q.pop();


        if(node == nullptr)
            continue;

        int h = node->h;
        for(Entry e : node->entries){
            int n = (e.a == nullptr) ? 0 : e.a->entries.size();

            outFile << "p:(" << e.p.x 
                    << ","   << e.p.y 
                    <<"),h:" << h
                    <<",n:"  << n
                    <<",cr:" << e.cr
                    <<";\n";

            q.push(e.a);
        }
    }
    outFile.close();
}


void printf_time(double seconds, std::string method_type, std::string test_type, int power){
    std::ostringstream fileName;
    fileName << "./resultados/" << method_type << "/" << test_type << "/time/" << power << ".csv";
    std::ofstream outFile(fileName.str());
    outFile << seconds << "\n";
    outFile.close();
}

void mtree_test(int power, int queries, int seed_sample, int seed_query, double r, MTree*(*constructor)(const std::vector<Point>&), std::string type){
    int size = (1 << power);
    MTree* mtree;
    std::vector<Point> points = random_sample_generator(size, seed_sample);

    printf("\nCreating MTree of %d elements\n", size);

    auto start = std::chrono::high_resolution_clock::now();
    mtree = (*constructor)(points);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken to create MTree: " << elapsed.count() << " seconds\n";
    printf_time(elapsed.count(), type, "mtree", power);


    if(power < 18)
        printf_mtree(mtree, power, type);


    std::ostringstream fileName;
    fileName << "./resultados/" << type << "/search/result/" << power << ".csv";
    std::ofstream outFile(fileName.str());

    double total_time = 0;
    std::vector<Point> query_points = random_sample_generator(queries, seed_query);
    for(int i=0; i<queries; i++){    
        Point q = query_points[i];

        start = std::chrono::high_resolution_clock::now();
        std::tuple<std::vector<Point>, int> ms = mtree_search(mtree, q, r);
        end = std::chrono::high_resolution_clock::now();
        elapsed = (end - start);
        total_time += elapsed.count();

        outFile << "size:"<<std::get<0>(ms).size() <<",ios:"<< std::get<1>(ms)<<";\n";
    }
    elapsed = end - start;
    std::cout << "Time taken for "<< queries <<" queries: " << total_time << " seconds\n";
    printf_time(total_time, type, "search", power);

    outFile.close();


    delete mtree;
}

int main(int argc, char **argv){
    srand(time(NULL));
    printf("\t=====  TEST  =====\n");

    int seed_sample = 333071598;
    int seed_query  = 948198108;

    int queries = (1 << 11); // se realizan 2^11 consultas, se pueden hacer menos
    double r = 0.02;         //radio de busqueda

    // modificar estos valores para hacer mas o menos test de cada metodo
    int max_power_cp = 25;
    int max_power_ss = 25;

    printf("\nMetodo CP \n");
    for(int power=10; power<=max_power_cp; power++)
        mtree_test(power, queries, seed_sample, seed_query, r, &mtree_create_cp, "cp");

    printf("\nMetodo SS \n");
    for(int power=10; power<=max_power_ss; power++)
        mtree_test(power, queries, seed_sample, seed_query, r, &mtree_create_ss, "ss");
}