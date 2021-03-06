//
// Created by mateus on 31/05/2020.
//

#include <cstring>
#include "tests/basic_tests.h"
#include "include/RK_Individual.h"

#define configuracao "inicial"

int *configuracaoInicial(char *arqIn);

void BasicTests::readNetworkTest(char *networkSourceFile) {
    Graph *g = new Graph();
    g->input_read(networkSourceFile);
}

void BasicTests::testRandomKeys(char *networkSourceFile) {
    Graph *g = new Graph();

    g->addChargeLevel(0.50, 1000, 0.06);
    g->addChargeLevel(1.00, 6760, 0.06);
    g->addChargeLevel(1.80, 1000, 0.06);

    g->input_read(networkSourceFile);
    g->defineModifiables();
    g->resetEdgesMarking();
    g->markOneDirectionInEdges();

    RK_Individual::criaCromossomos(g);

    Random_keys *rd = new Random_keys(100, 1000);

    rd->geraPopAleatoria(g);

//    rd->geraPopAleatoriaConfInicial(g, configuracaoInicial(networkSourceFile),
//                                    g->getEdgesSizes() / 2 - (g->getVerticesSize() - 1));

    int melhorGeracao = rd->avancaGeracoes(g);

    RK_Individual *best = rd->getPopAtual().at(rd->getTamPopulacao() - 1);

    printf("        %.3f  &  ", 100 * 1000 * best->getActiveLoss());
    printf("        %.3f  &   ", g->minVoltage());
    printf("        %d    &   ", melhorGeracao);
}

void BasicTests::print_tree(const string savefolder, Graph *g) {
    system(("mkdir " + savefolder + " -p").c_str());
    system(("mkdir " + savefolder + "/Originals -p").c_str());

    string savename_e_aux = savefolder + "/original_e_tree.csv";
    string savename_v_aux = savefolder + "/original_v_tree.csv";
    FILE *output_file_edges_aux = fopen((savename_e_aux).c_str(), "w");
    FILE *output_file_vertices_aux = fopen((savename_v_aux).c_str(), "w");

    for (Vertex *v_aux = g->get_verticesList(); v_aux != NULL; v_aux = v_aux->getNext()) {
        for (Edge *e = v_aux->getEdgesList(); e != NULL; e = e->getNext()) {
            if (e->isClosed())
                fprintf(output_file_edges_aux, "%2d, %2d, %4.6f, %4.6f, %4.6f, %4.6f, %4.6f, %4.6f\n",
                        e->getOrigin()->getID(), e->getDestiny()->getID(), e->getActiveLoss(), e->getActivePowerFlow(),
                        e->getReactiveLoss(), e->getReactivePowerFlow(), e->getResistance(), e->getReactance());
        }
        fprintf(output_file_vertices_aux, "%2d, %1d, %4.6f, %4.6f, %4.6f\n",
                v_aux->getID(), (int) v_aux->getCapacitors().size(), v_aux->getActivePower(), v_aux->getReactivePower(),
                v_aux->getVoltage());
    }
    fclose(output_file_vertices_aux);
    fclose(output_file_edges_aux);
    system(("python3 printTree.py " + savefolder).c_str()); // plota grafico da rede
}

void BasicTests::Capacitor_Test_GA(string input_file) {
    string name = input_file;

    Graph *graph = new Graph();

    /// Adiciona níveis de carga (IVO)
    graph->addChargeLevel(0.50, 1000, 0.06);
    graph->addChargeLevel(1.00, 6760, 0.06);
    graph->addChargeLevel(1.80, 1000, 0.06);
//    graph->addChargeLevel( 2.45, 1000, 0.06 );

    graph->input_read(name);

    ///-----------necessario ter isso agora para funcao objetivo otimizada------------------
    graph->defineModifiables();
    graph->resetEdgesMarking();
    graph->markOneDirectionInEdges();
    RK_Individual::criaCromossomos(graph);
    ///-----------necessario ter isso agora para funcao objetivo otimizada------------------

    graph->createCapacitorType(1, 300,
                               3);                  // Colocar o valor do capacitor como um parametro de leitura pra ser convertido pelo Pb

    Random_keys *rd = new Random_keys(100, 1000);           /// numero de individuos da populacao,numero de geracoes
    rd->geraPopAleatoria(graph);                          /// populacao inicial gerada de forma aleatoria
    rd->avancaGeracoes(graph);                          /// faz cruzamentos e mutacoes para gerar individuos da nova populacao

    RK_Individual *best = rd->getPopAtual().at(
            rd->getTamPopulacao() - 1); /// melhor individuo eh o ultimo (menor perda)
    best->calculate_fitness(graph,
                            true); /// abre e fecha os arcos correspondentes do grafo *g para calcular funcao Objetivo

    vector<int> openSwitches;
    printf("\n\n\nMELHOR INDIVIDUO FINAL: \n");
    for (Vertex *v = graph->get_verticesList(); v != NULL; v = v->getNext()) {
        for (Edge *e = v->getEdgesList(); e != NULL; e = e->getNext()) {
            Edge *e_reverse = graph->findEdge(e->getDestiny()->getID(), e->getOrigin()->getID());
            if (!e->isClosed() && !e_reverse->isClosed())
                openSwitches.push_back(e->getID());
        }
    }
    sort(openSwitches.begin(), openSwitches.end());

    cout << "Chaves Abertas:" << endl << "| ";
    for (uint i = 0; i < openSwitches.size(); i += 2) cout << openSwitches.at(i) << " | ";
    cout << endl;
    printf("--------------------------------------------------------------------\n");
    printf(" Load lvl \t| \t Losses\t\t| \t     Cost \t\t| \t Min Tension \n");
    printf("--------------------------------------------------------------------\n");
    for (int i = 0; i < 3; i++) {
        graph->evaluateLossesAndFlows(1e-12, i);
        graph->show_losses(100 * 1000 * graph->getLosses()[0], graph->minVoltage(), i);
    }
    graph->show_losses(graph->getTotalLoss()[0], graph->minVoltage(), -1);

    print_tree("out/" + name, graph);
}

void BasicTests::brandaoTest(string input_file) {
    srand(time(NULL));

    string savename = input_file;
    savename.erase(savename.begin(), savename.begin() + 7);
    savename.erase(savename.length() - 2, savename.length() - 1);

//    testeEntradas(savename);//perda total e tensao minima para cada configuration para compara com a tese do leonardo willer

//    Capacitor_Test_Ivo(savename);
    Capacitor_Test_GA(savename);
}

void BasicTests::testRandomKeysAG(string networkSourceFile, int populationSize, int numGenerations) {
    Graph *g = new Graph();
    g->input_read(networkSourceFile);
    g->defineModifiables();
    g->markOneDirectionInEdges();

    RK_Individual::criaCromossomos(g);

    Random_keys *randomKeys = new Random_keys(populationSize, numGenerations);
    randomKeys->geraPopAleatoria(g);
    randomKeys->ordenaPopulacaoAtual(g);
    printf("best individual: %f",
           100 * 1000 * randomKeys->getPopAtual().at(randomKeys->getTamPopulacao() - 1)->getActiveLoss());
}

void BasicTests::testCalculationOfObjectFunctionOpmized(string networkSourceFile) {
    Graph *g = new Graph();
    g->input_read(networkSourceFile);
    g->defineModifiables();
    g->markOneDirectionInEdges();

    for (Vertex *v = g->get_verticesList(); v != NULL; v = v->getNext()) {
        printf("| idarv: %d", v->getIdTree());
        for (Edge *e = v->getEdgesList(); e != NULL; e = e->getNext()) {
            printf("- e->closed: %d\n", e->isClosed());
        }
    }

    RK_Individual::criaCromossomos(g);

    RK_Individual *rkIndividual = new RK_Individual(RK_Individual::cromossomos.size());
    rkIndividual->generate_random_weights();
    rkIndividual->calculaFuncaoObjetivoOtimizado(g);
    printf("random weights individual objective funcion: %f", 100 * 1000 * rkIndividual->getActiveLoss());
}

int *configuracaoInicial(char *arqIn) {
    int *ids;
    if (strcmp(arqIn, "networks/sis33.m") == 0) {

        ids = new int[5];

        //CONFIGURACAO INICIAL
        if (strcmp(configuracao, "inicial") == 0) {
            ids[0] = 33;
            ids[1] = 34;
            ids[2] = 35;
            ids[3] = 36;
            ids[4] = 37;
        }
        //CONFIGURACAO DA LITERATURA
        if (strcmp(configuracao, "literatura1") == 0) {
            ids[0] = 7;
            ids[1] = 10;
            ids[2] = 14;
            ids[3] = 32;
            ids[4] = 37;
        }
        //ABORDAGEM ARSD
        if (strcmp(configuracao, "ARSD") == 0) {
            ids[0] = 7;
            ids[1] = 9;
            ids[2] = 14;
            ids[3] = 32;
            ids[4] = 37;
        }

        return ids;
    }

    if (strcmp(arqIn, "networks/sis33modif.m") == 0) {

        ids = new int[5];

        //CONFIGURACAO INICIAL
        if (strcmp(configuracao, "inicial") == 0) {
            ids[0] = 33;
            ids[1] = 34;
            ids[2] = 35;
            ids[3] = 36;
            ids[4] = 37;
        }
        //CONFIGURACAO DA LITERATURA
        if (strcmp(configuracao, "literatura1") == 0) {
            ids[0] = 7;
            ids[1] = 10;
            ids[2] = 14;
            ids[3] = 28;
            ids[4] = 36;
        }
        //ABORDAGEM ARSD
        if (strcmp(configuracao, "ARSD") == 0) {
            ids[0] = 7;
            ids[1] = 10;
            ids[2] = 14;
            ids[3] = 16;
            ids[4] = 28;
        }

        return ids;
    }

    if (strcmp(arqIn, "networks/sis119.m") == 0) {

        ids = new int[15];
        //CONFIGURACAO INICIAL
        if (strcmp(configuracao, "inicial") == 0) {
            ids[0] = 119;
            ids[1] = 120;
            ids[2] = 121;
            ids[3] = 122;
            ids[4] = 123;
            ids[5] = 124;
            ids[6] = 125;
            ids[7] = 126;
            ids[8] = 127;
            ids[9] = 128;
            ids[10] = 129;
            ids[11] = 130;
            ids[12] = 131;
            ids[13] = 132;
            ids[14] = 133;
        }
        //CONFIGURACAO DA LITERATURA
        if (strcmp(configuracao, "literatura1") == 0) {
            ids[0] = 24;
            ids[1] = 27;
            ids[2] = 35;
            ids[3] = 40;
            ids[4] = 43;
            ids[5] = 52;
            ids[6] = 59;
            ids[7] = 72;
            ids[8] = 75;
            ids[9] = 96;
            ids[10] = 99;
            ids[11] = 110;
            ids[12] = 123;
            ids[13] = 130;
            ids[14] = 131;
        }
        //ABORDAGEM ARSD
        if (strcmp(configuracao, "ARSD") == 0) {
            ids[0] = 24;
            ids[1] = 26;
            ids[2] = 35;
            ids[3] = 40;
            ids[4] = 43;
            ids[5] = 51;
            ids[6] = 61;
            ids[7] = 72;
            ids[8] = 75;
            ids[9] = 96;
            ids[10] = 98;
            ids[11] = 110;
            ids[12] = 122;
            ids[13] = 130;
            ids[14] = 131;
        }

        /*
        esse arquivo tem na verdade 132 arcos ao inves de 133
        dessa forma os ids dos arcos abertos deve ser subtraido 1.
        */
        for (int i = 0; i < 15; i++)
            ids[i]--;

        return ids;
    }
    if (strcmp(arqIn, "networks/sis83.m") == 0) {

        ids = new int[13];

        //CONFIGURACAO INICIAL
        if (strcmp(configuracao, "inicial") == 0) {
            ids[0] = 84;
            ids[1] = 85;
            ids[2] = 86;
            ids[3] = 87;
            ids[4] = 88;
            ids[5] = 89;
            ids[6] = 90;
            ids[7] = 91;
            ids[8] = 92;
            ids[9] = 93;
            ids[10] = 94;
            ids[11] = 95;
            ids[12] = 96;
        }
        //CONFIGURACAO DA LITERATURA
        if (strcmp(configuracao, "literatura1") == 0) {
            ids[0] = 7;
            ids[1] = 13;
            ids[2] = 34;
            ids[3] = 39;
            ids[4] = 42;
            ids[5] = 55;
            ids[6] = 62;
            ids[7] = 72;
            ids[8] = 83;
            ids[9] = 86;
            ids[10] = 89;
            ids[11] = 90;
            ids[12] = 92;
        }

        return ids;
    }
    if (strcmp(arqIn, "networks/sis83modif.m") == 0) {

        ids = new int[13];

        //CONFIGURACAO INICIAL
        if (strcmp(configuracao, "inicial") == 0) {
            ids[0] = 84;
            ids[1] = 85;
            ids[2] = 86;
            ids[3] = 87;
            ids[4] = 88;
            ids[5] = 89;
            ids[6] = 90;
            ids[7] = 91;
            ids[8] = 92;
            ids[9] = 93;
            ids[10] = 94;
            ids[11] = 95;
            ids[12] = 96;
        }
        //CONFIGURACAO DA LITERATURA
        if (strcmp(configuracao, "literatura1") == 0) {
            ids[0] = 7;
            ids[1] = 13;
            ids[2] = 33;
            ids[3] = 38;
            ids[4] = 42;
            ids[5] = 63;
            ids[6] = 72;
            ids[7] = 83;
            ids[8] = 84;
            ids[9] = 86;
            ids[10] = 89;
            ids[11] = 90;
            ids[12] = 92;
        }
        if (strcmp(configuracao, "literatura2") == 0) {
            ids[0] = 7;
            ids[1] = 13;
            ids[2] = 34;
            ids[3] = 38;
            ids[4] = 42;
            ids[5] = 63;
            ids[6] = 72;
            ids[7] = 83;
            ids[8] = 84;
            ids[9] = 86;
            ids[10] = 89;
            ids[11] = 90;
            ids[12] = 92;
        }
        if (strcmp(configuracao, "ARSD") == 0) {
            ids[0] = 7;
            ids[1] = 13;
            ids[2] = 34;
            ids[3] = 39;
            ids[4] = 42;
            ids[5] = 63;
            ids[6] = 72;
            ids[7] = 83;
            ids[8] = 84;
            ids[9] = 86;
            ids[10] = 89;
            ids[11] = 90;
            ids[12] = 92;
        }

        return ids;
    }

    ids = new int[1];
    ids[0] = 0;
    return ids;
}