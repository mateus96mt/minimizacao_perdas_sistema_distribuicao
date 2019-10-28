#include <stdio.h>
#include <cstdlib>
#include "Graph.h"
#include "Random_keys.h"
#include "time.h"
#include <algorithm>
#include <math.h>
#include <string.h>

//#define input_file "inputs/sist33barras_Yang.m"
//#define input_file "inputs/sist33barras_Yang-modificado.m"
//#define input_file "inputs/SISTEMA119s2.m"
//#define input_file "inputs/sist135barras.m"
//#define input_file "inputs/sist215barras.m"
//#define input_file "inputs/SISTEMA83_TAIWAN.m"
//#define input_file "inputs/SISTEMA83_TAIWAN_modificado.m"
#define input_file "inputs/sist69barras.m"
//#define input_file "inputs/Caract_Sistem_85.m"

#define configuration "inicial"
//#define configuration "literatura1"
//#define configuration "literatura2"
//#define configuration "ARSD"

void openSwitches(Graph *g, int *ids, int n);
void defineConfiguration(Graph *g);
void testeEntradas();

void print_tree(const string savefolder, Graph *g );
void cap_combination1(const string savefolder, Graph *g, Individual *individual);
void cap_combination3(const string savefolder, Graph *g, Individual *individual);
void Capacitor_Test_GA(const string savename);
void Capacitor_Test_Ivo(const string savename);

int main(){
    srand(time(NULL));

    string savename = input_file;
    savename.erase ( savename.begin()   , savename.begin()+7 );
    savename.erase ( savename.length()-2, savename.length()-1 );

//    testeEntradas();//perda total e tensao minima para cada configuration para compara com a tese do leonardo willer

//    Capacitor_Test_Ivo(savename);
    Capacitor_Test_GA(savename);
}


void Capacitor_Test_Ivo(const string savename){
    string name = input_file;
    Graph *graph = new Graph();
    Graph *last_graph = new Graph();

    /// Adiciona níveis de carga
    graph->addChargeLevel( 0.50, 1000, 0.06 );
    graph->addChargeLevel( 1.00, 6760, 0.06 );
    graph->addChargeLevel( 2.45, 1000, 0.06 );

    graph->input_read(name);
    graph->createCapacitorType(1, 200, 4);                  // Colocar o valor do capacitor como um parametro de leitura pra ser convertido pelo Pb

    defineConfiguration(graph);

    graph->defineFlows();

    printf("--------------------------------------------------------------------\n");
    printf(" Load lvl \t| \t Losses\t\t| \t     Cost \t\t| \t Min Tension \n");
    printf("--------------------------------------------------------------------\n");
    for(int i = 0; i < 3; i++) {
        graph->evaluateLossesAndFlows(1e-12, i);
        graph->show_losses(100 * 1000 * graph->getLosses()[0], graph->minVoltage(), i );
    }
    graph->show_losses( graph->getTotalLoss()[0], graph->minVoltage(), -1 );

    graph->capacitor_allocation();


//                                                                        // 0
//    graph->findVertex(62)->addCapacitor( graph->getCapacitorType(1) );  // 1
//    graph->findVertex(62)->addCapacitor( graph->getCapacitorType(1) );  // 2
//    graph->findVertex(62)->addCapacitor( graph->getCapacitorType(1) );  // 3
//    graph->findVertex(65)->addCapacitor( graph->getCapacitorType(1) );  // 4
//    graph->findVertex(60)->addCapacitor( graph->getCapacitorType(1) );  // 5
//    graph->findVertex(63)->addCapacitor( graph->getCapacitorType(1) );  // 6
//    graph->findVertex(22)->addCapacitor( graph->getCapacitorType(1) );  // 7
//    graph->findVertex(65)->addCapacitor( graph->getCapacitorType(1) );  // 8
//    graph->findVertex(13)->addCapacitor( graph->getCapacitorType(1) );  // 9

//                                                                        // 0
//    graph->findVertex(65)->addCapacitor( graph->getCapacitorType(1) );  // 1
//    graph->findVertex(64)->addCapacitor( graph->getCapacitorType(1) );  // 2
//    graph->findVertex(64)->addCapacitor( graph->getCapacitorType(1) );  // 3
//    graph->findVertex(61)->addCapacitor( graph->getCapacitorType(1) );  // 4
//    graph->findVertex(61)->addCapacitor( graph->getCapacitorType(1) );  // 5
//    graph->findVertex(61)->addCapacitor( graph->getCapacitorType(1) );  // 6
//    graph->findVertex(61)->addCapacitor( graph->getCapacitorType(1) );  // 7
//    graph->findVertex(61)->addCapacitor( graph->getCapacitorType(1) );  // 8
//    graph->findVertex(61)->addCapacitor( graph->getCapacitorType(1) );  // 9
//    graph->findVertex(61)->addCapacitor( graph->getCapacitorType(1) );  // 10
//    graph->findVertex(27)->addCapacitor( graph->getCapacitorType(1) );  // 11

    printf("--------------------------------------------------------------------\n");
    printf(" Load lvl \t| \t Losses\t\t| \t     Cost \t\t| \t Min Tension \n");
    printf("--------------------------------------------------------------------\n");
    for(int i = 0; i < 3; i++) {
        graph->evaluateLossesAndFlows(1e-12, i);
        graph->show_losses(100 * 1000 * graph->getLosses()[0], graph->minVoltage(), i );
    }
    graph->show_losses( graph->getTotalLoss()[0], graph->minVoltage(), -1 );

    print_tree("out/" + savename + "_IVO", graph );
}

void Capacitor_Test_GA(const string savename){
    string name = input_file;

    Graph *graph = new Graph();

    /// Adiciona níveis de carga
//    graph->addChargeLevel( 0.50, 1000, 0.06 );
    graph->addChargeLevel( 1.00, 6760, 0.06 );
    graph->addChargeLevel( 2.45, 1000, 0.06 );

    graph->input_read(name);
    graph->createCapacitorType(1, 200, 4);                  // TODO colocar o valor do capacitor como um parametro de leitura pra ser convertido pelo Pb

    Random_keys *rd = new Random_keys(100, 1000);       /// numero de individuos da populacao,numero de geracaoes
    rd->generatePopulation(graph);                          /// populacao inicial gerada de forma aleatoria
    rd->forwardGenerations(graph);                          /// faz cruzamentos e mutacoes para gerar individuos da nova populacao

    Individual *best = rd->getCurrentPop().at(rd->getPopulationSize()-1); /// melhor individuo eh o ultimo (menor perda)
    best->calculate_fitness(graph); /// abre e fecha os arcos correspondentes do grafo *g para calcular funcao Objetivo

    vector<int> openSwitches;
    printf("\n\n\nMELHOR INDIVIDUO FINAL: \n");
    for(Vertex *v = graph->get_verticesList(); v!=NULL; v = v->getNext()){
        for(Edge *e = v->getEdgesList(); e != NULL; e = e->getNext()){
            Edge *e_reverse = graph->findEdge(e->getDestiny()->getID(), e->getOrigin()->getID());
            if( !e->isClosed() && !e_reverse->isClosed() )
                openSwitches.push_back(e->getID());
        }
    }
    sort(openSwitches.begin(), openSwitches.end());

    cout << "Chaves Abertas:" << endl << "| ";
    for( uint i=0; i<openSwitches.size();i+=2)   cout << openSwitches.at(i) << " | ";
    cout << endl;
    graph->show_losses( 100 * 1000 * best->getActiveLoss(), graph->minVoltage() ) ;

    print_tree("out/" + savename, graph );
//    cap_combination1( "out/" + savename, graph, best );
//    cap_combination2( "out/" + savename, graph, best );
//    cap_combination3( "out/" + savename, graph, best );
}

void print_tree(const string savefolder, Graph *g ) {
    system( ("mkdir " + savefolder + " -p").c_str() );
    system( ("mkdir " + savefolder + "/Originals -p").c_str() );

    string savename_e_aux = savefolder + "/original_e_tree.csv";
    string savename_v_aux = savefolder + "/original_v_tree.csv";
    FILE *output_file_edges_aux = fopen((savename_e_aux).c_str(), "w");
    FILE *output_file_vertices_aux = fopen((savename_v_aux).c_str(), "w");

    for (Vertex *v_aux = g->get_verticesList(); v_aux != NULL; v_aux = v_aux->getNext()) {
        for (Edge *e = v_aux->getEdgesList(); e != NULL; e = e->getNext()) {
            if ( e->isClosed() )
                fprintf(output_file_edges_aux, "%2d, %2d, %4.6f, %4.6f, %4.6f, %4.6f, %4.6f, %4.6f\n",
                        e->getOrigin()->getID(), e->getDestiny()->getID(), e->getActiveLoss(),e->getActivePowerFlow(), e->getReactiveLoss(),e->getReactivePowerFlow(), e->getResistance(), e->getReactance() );
        }
        fprintf(output_file_vertices_aux, "%2d, %1d, %4.6f, %4.6f, %4.6f\n",
                v_aux->getID(), v_aux->getCapacitors().size(), v_aux->getActivePower(), v_aux->getReactivePower(), v_aux->getVoltage() );
    }
    fclose(output_file_vertices_aux);
    fclose(output_file_edges_aux);
    system( ("python3 printTree.py " + savefolder ).c_str() ); // plota grafico da rede
}

void cap_combination1(const string savefolder, Graph *g, Individual *best ) {
    int cap1, cap2 = 0, cap3 = 0;

    system( ("mkdir " + savefolder + " -p").c_str() );
    system( ("mkdir " + savefolder + "/caps1 -p").c_str() );
    system( ("mkdir " + savefolder + "/1Caps_best -p").c_str() );

    string savename_cap = savefolder + "/teste_capacitor_1caps.csv";
    FILE *output_file_cap = fopen((savename_cap).c_str(), "w");

    for (Vertex *v_cap1 = g->get_verticesList(); v_cap1 != NULL; v_cap1 = v_cap1->getNext()) {
        cap1 = v_cap1->getID();
        best->calculate_fitness_cap(g, v_cap1);

        fprintf(output_file_cap, "%3d, %3d, %3d, %4.6f, %4.6f\n",
                v_cap1->getID(), cap2, cap3, 100 * 1000 * best->getActiveLoss(), g->minVoltage());
        string namingCaps = to_string(cap1) + "-" + to_string(cap2) + "-" + to_string(cap3);
        string savename_e_aux = savefolder + "/caps1/graph_cap" + namingCaps + "_e_tree.csv";
        string savename_v_aux = savefolder + "/caps1/graph_cap" + namingCaps + "_v_tree.csv";
        FILE *output_file_edges_aux = fopen((savename_e_aux).c_str(), "w");
        FILE *output_file_vertices_aux = fopen((savename_v_aux).c_str(), "w");

        for (Vertex *v_aux = g->get_verticesList(); v_aux != NULL; v_aux = v_aux->getNext()) {
            for (Edge *e = v_aux->getEdgesList(); e != NULL; e = e->getNext()) {
                if (e->isClosed() == true)
                    fprintf(output_file_edges_aux, "%2d, %2d, %4.6f, %4.6f, %4.6f, %4.6f, %4.6f, %4.6f\n",
                            e->getOrigin()->getID(), e->getDestiny()->getID(), e->getActiveLoss(),e->getActivePowerFlow(), e->getReactiveLoss(),e->getReactivePowerFlow(), e->getResistance(), e->getReactance() );
            }
            fprintf(output_file_vertices_aux, "%2d, %1d, %4.6f, %4.6f, %4.6f\n",
                    v_aux->getID(), v_aux->getCapacitors().size() > 0, v_aux->getActivePower(), v_aux->getReactivePower(), v_aux->getVoltage() );
        }
        fclose(output_file_vertices_aux);
        fclose(output_file_edges_aux);
        v_cap1->rm_all_capacitors();
    }
    fclose(output_file_cap);

    system( ( "python3 findBests.py " + savename_cap ).c_str() ); // plota grafico da rede
}

void cap_combination3(const string savefolder, Graph *g, Individual *best ) {
    int cap1, cap2 = 0, cap3 = 0;

    system( ("mkdir " + savefolder + " -p").c_str() );
    system( ("mkdir " + savefolder + "/caps3 -p").c_str() );
    system( ("mkdir " + savefolder + "/3Caps_best -p").c_str() );
    string savename_cap =  savefolder + "/teste_capacitor_3caps.csv";
    FILE *output_file_cap = fopen((savename_cap).c_str(), "w");

    for (Vertex *v_cap1 = g->get_verticesList(); v_cap1 != NULL; v_cap1 = v_cap1->getNext()) {
        cap1 = v_cap1->getID();
        for (Vertex *v_cap2 = v_cap1->getNext(); v_cap2 != NULL; v_cap2 = v_cap2->getNext()) {
            cap2 = v_cap2->getID();
            for (Vertex *v_cap3 = v_cap2->getNext(); v_cap3 != NULL; v_cap3 = v_cap3->getNext()) {
                cap3 = v_cap3->getID();
                best->calculate_fitness_cap(g, v_cap1, v_cap2, v_cap3);

                fprintf(output_file_cap, "%3d, %3d, %3d, %4.6f, %4.6f\n",
                        v_cap1->getID(), cap2, cap3, 100 * 1000 * best->getActiveLoss(), g->minVoltage());
                string namingCaps = to_string(cap1) + "-" + to_string(cap2) + "-" + to_string(cap3);
                string savename_e_aux = savefolder + "/caps3/graph_cap" + namingCaps + "_e_tree.csv";
                string savename_v_aux = savefolder + "/caps3/graph_cap" + namingCaps + "_v_tree.csv";
                FILE *output_file_edges_aux = fopen((savename_e_aux).c_str(), "w");
                FILE *output_file_vertices_aux = fopen((savename_v_aux).c_str(), "w");

                for (Vertex *v_aux = g->get_verticesList(); v_aux != NULL; v_aux = v_aux->getNext()) {
                    for (Edge *e = v_aux->getEdgesList(); e != NULL; e = e->getNext()) {
                        if (e->isClosed() == true)
                            fprintf(output_file_edges_aux, "%2d, %2d, %4.6f, %4.6f, %4.6f, %4.6f, %4.6f, %4.6f\n",
                                    e->getOrigin()->getID(), e->getDestiny()->getID(), e->getActiveLoss(),e->getActivePowerFlow(), e->getReactiveLoss(),e->getReactivePowerFlow(), e->getResistance(), e->getReactance() );
                    }
                    fprintf(output_file_vertices_aux, "%2d, %1d, %4.6f, %4.6f, %4.6f\n",
                            v_aux->getID(), v_aux->getCapacitors().size() > 0, v_aux->getActivePower(), v_aux->getReactivePower(), v_aux->getVoltage() );
                }
                fclose(output_file_vertices_aux);
                fclose(output_file_edges_aux);
                v_cap1->rm_all_capacitors();
                v_cap2->rm_all_capacitors();
                v_cap3->rm_all_capacitors();
            }
        }
    }
    fclose(output_file_cap);

    system( ("python3 findBests.py " + savename_cap).c_str() ); // plota grafico da rede
}

void testeEntradas(){
    char nome[] = input_file;
    Graph *g = new Graph();
    g->input_read(nome);

    defineConfiguration(g);

    g->defineFlows();
    g->evaluateLossesAndFlows(1e-8);

    printf("\ntensao minima: %.5f (p.u)", g->minVoltage());
    printf("\nperdaTotal: %.5f (kW)", 100*1000* g->getLosses()[0]);
    printf("\neh Conexo? %d", g->isConected());
    printf("\neh arvore? %d\n\n\n", g->isTree());
}

void defineConfiguration(Graph *g){

    if( input_file == "inputs/sist33barras_Yang.m" ){
        int ids[5];

        //CONFIGURACAO INICIAL
        if(configuration == "inicial"){
            ids[0] = 33; ids[1] = 34; ids[2] = 35; ids[3] = 36; ids[4] = 37;
        }
        //CONFIGURACAO DA LITERATURA
        if(configuration == "literatura1"){
            ids[0] = 7; ids[1] = 10; ids[2] = 14; ids[3] = 32; ids[4] = 37;
        }
        //ABORDAGEM ARSD
        if(configuration == "ARSD"){
            ids[0] = 7; ids[1] = 9; ids[2] = 14; ids[3] = 32; ids[4] = 37;
        }

        openSwitches(g, ids, 5);
    }

    if( input_file == "inputs/sist33barras_Yang-modificado.m"){
        int ids[5];

        //CONFIGURACAO INICIAL
        if(configuration == "inicial"){
            ids[0] = 33; ids[1] = 34; ids[2] = 35; ids[3] = 36; ids[4] = 37;
        }
        //CONFIGURACAO DA LITERATURA
        if(configuration == "literatura1"){
            ids[0] = 7; ids[1] = 10; ids[2] = 14; ids[3] = 28; ids[4] = 36;
        }
        //ABORDAGEM ARSD
        if(configuration == "ARSD"){
            ids[0] = 7; ids[1] = 10; ids[2] = 14; ids[3] = 16; ids[4] = 28;
        }

        openSwitches(g, ids, 5);
    }

    if(input_file == "inputs/SISTEMA119s2.m"){
        int ids[15];
        //CONFIGURACAO INICIAL
        if(configuration == "inicial"){
            ids[0] = 119; ids[1] = 120; ids[2] = 121; ids[3] = 122; ids[4] = 123;
            ids[5] = 124; ids[6] = 125; ids[7] = 126; ids[8] = 127; ids[9] = 128;
            ids[10] = 129; ids[11] = 130; ids[12] = 131; ids[13] = 132; ids[14] = 133;
        }
        //CONFIGURACAO DA LITERATURA
        if(configuration == "literatura1"){
            ids[0] = 24; ids[1] = 27; ids[2] = 35; ids[3] = 40; ids[4] = 43;
            ids[5] = 52; ids[6] = 59; ids[7] = 72; ids[8] = 75; ids[9] = 96;
            ids[10] = 99; ids[11] = 110; ids[12] = 123; ids[13] = 130; ids[14] = 131;
        }
        //ABORDAGEM ARSD
        if(configuration == "ARSD"){
            ids[0] = 24; ids[1] = 26; ids[2] = 35; ids[3] = 40; ids[4] = 43;
            ids[5] = 51; ids[6] = 61; ids[7] = 72; ids[8] = 75; ids[9] = 96;
            ids[10] = 98; ids[11] = 110; ids[12] = 122; ids[13] = 130; ids[14] = 131;
        }

        /*
        esse arquivo tem na verdade 132 arcos ao inves de 133 dessa forma os ids dos arcos abertos deve ser subtraido 1.
        */
        for(int i=0; i<15; i++)
            ids[i]--;

        openSwitches(g, ids, 15);
    }
    if( input_file == "inputs/SISTEMA83_TAIWAN.m" ){
        int ids[13];

        //CONFIGURACAO INICIAL
        if(configuration == "inicial"){
            ids[0] = 84; ids[1] = 85; ids[2] = 86; ids[3] = 87; ids[4] = 88;
            ids[5] = 89; ids[6] = 90; ids[7] = 91; ids[8] = 92; ids[9] = 93;
            ids[10] = 94; ids[11] = 95; ids[12] = 96;
        }
        //CONFIGURACAO DA LITERATURA
        if(configuration == "literatura1"){
            ids[0] = 7; ids[1] = 13; ids[2] = 34; ids[3] = 39; ids[4] = 42;
            ids[5] = 55; ids[6] = 62; ids[7] = 72; ids[8] = 83; ids[9] = 86;
            ids[10] = 89; ids[11] = 90; ids[12] = 92;
        }
        openSwitches(g, ids, 13);
    }
    if( input_file == "inputs/SISTEMA83_TAIWAN_modificado.m" ){
        int ids[13];

        //CONFIGURACAO INICIAL
        if(configuration == "inicial"){
            ids[0] = 84; ids[1] = 85; ids[2] = 86; ids[3] = 87; ids[4] = 88;
            ids[5] = 89; ids[6] = 90; ids[7] = 91; ids[8] = 92; ids[9] = 93;
            ids[10] = 94; ids[11] = 95; ids[12] = 96;
        }
        //CONFIGURACAO DA LITERATURA
        if(configuration == "literatura1"){
            ids[0] = 7; ids[1] = 13; ids[2] = 33; ids[3] = 38; ids[4] = 42;
            ids[5] = 63; ids[6] = 72; ids[7] = 83; ids[8] = 84; ids[9] = 86;
            ids[10] = 89; ids[11] = 90; ids[12] = 92;
        }
        if(configuration == "literatura2"){
            ids[0] = 7; ids[1] = 13; ids[2] = 34; ids[3] = 38; ids[4] = 42;
            ids[5] = 63; ids[6] = 72; ids[7] = 83; ids[8] = 84; ids[9] = 86;
            ids[10] = 89; ids[11] = 90; ids[12] = 92;
        }
        if(configuration == "ARSD"){
            ids[0] = 7; ids[1] = 13; ids[2] = 34; ids[3] = 39; ids[4] = 42;
            ids[5] = 63; ids[6] = 72; ids[7] = 83; ids[8] = 84; ids[9] = 86;
            ids[10] = 89; ids[11] = 90; ids[12] = 92;
        }

        openSwitches(g, ids, 13);
    }
    if( input_file == "inputs/sist69barras.m" ){
        int ids[5];

        ids[0] = 69; ids[1] = 70; ids[2] = 71; ids[3] = 72; ids[4] = 73;

        openSwitches(g, ids, 5);
    }
}

void openSwitches(Graph *g, int *ids, int n){
    Edge *a;
    for(int i=0; i<n; i++){
        a = g->findEdge(ids[i]);
        a->setSwitch(false);
        a = g->findEdge(a->getDestiny()->getID(), a->getOrigin()->getID());
        a->setSwitch(false);
    }
}
