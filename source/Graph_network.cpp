#include "include/Graph_network.h"
#include <fstream>
#include <math.h>
#include "time.h"
#include <algorithm>

using namespace std;

Graph_network::Graph_network(){

    this->listaNos = NULL;
    this->numeroNos = 0;
    this->numeroArcos = 0;
    this->perdaAtivaTotal = 0.0;
    this->perdaReativaTotal = 0.0;
    this->n_marcados = 0;
}

Graph_network::~Graph_network(){
    Vertex *aux = new Vertex(0);
    for(Vertex *no=this->listaNos; no != NULL; no=no->getProxNo()){
        delete aux;
        for(Edge *arco=no->getListaArcos(); arco != NULL; arco=arco->getProxArco()){
            delete arco;
        }
        aux = no;
    }
    delete aux;
}

Vertex *Graph_network::buscaNo(int id){

    Vertex *no = listaNos;
    while(no!=NULL){
        if(no->getID() == id)
            return no;
        no = no->getProxNo();
    }
    return NULL;

}

Edge *Graph_network::buscaArco(int id){
    for(Vertex *no=getListaNos(); no != NULL; no=no->getProxNo()){
        for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){
            if(a->getID() == id)
                return a;
        }
    }
    return NULL;
}

Edge *Graph_network::buscaArco(int idOrigem, int idDestino){
    for(Vertex *no=getListaNos(); no != NULL; no=no->getProxNo()){
        for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){
            if(a->getNoOrigem()->getID() == idOrigem && a->getNoDestino()->getID() == idDestino)
                return a;
        }
    }
    return NULL;
}

void Graph_network::insereNo(int id, double potAtiva, double potReativa, double voltagem){

    Vertex *no=new Vertex(id);
    no->setPotAtiva(potAtiva);
    no->setPotReativa(potReativa);
    no->setVoltagem(voltagem);

    //insere No no inicio da lista
    no->setProxNo(listaNos);
    this->listaNos=no;

    this->numeroNos++;

}

void Graph_network::insereArco(int idOrigem, int idDestino, int id, double res, double reat, bool chave){
    Vertex *noOrigem = buscaNo(idOrigem);
    Vertex *noDestino = buscaNo(idDestino);

    Edge *novaArco = new Edge(id);
    novaArco->setNoDestino(noDestino);
    novaArco->setNoOrigem(noOrigem);

    //insere Arco no inicio da lista
    novaArco->setProxArco(noOrigem->getListaArcos());
    noOrigem->setListaArcos(novaArco);

    novaArco->setResistencia(res);
    novaArco->setReatancia(reat);
    novaArco->setChave(chave);


    noOrigem->setGrauSaida(noOrigem->getGrauSaida()+1);
    noDestino->setGrauEntrada(noDestino->getGrauEntrada()+1);

    this->numeroArcos++;
}

void Graph_network::leEntrada(char nome[])
{

    int idArco = 0;
    double VB, PB, ZB, fator_MW = 1e-3;
    double carga, resistencia, reatancia, potencia_reativa, voltagem;

    ifstream entrada;
    entrada.open(nome);
    string aux;

    int n_col_nos=13, n_col_arestas=12;//numero de colunas do arquivo de entrada

    do{
        entrada >> aux;
    }while(aux != "Vb");
    entrada >> aux;
    entrada >> VB;
    entrada >> aux;entrada >> aux;
    entrada >> PB;
    entrada >> aux;entrada >> aux;
    entrada >> ZB;


    entrada.close();
    entrada.open(nome);
    int idNo, idOrig, idDest;


    do{
        entrada >> aux;
    }while(aux != "num_nos");///pula lixo ate chegar na parte de informacoes do no

    int num_nos;
    entrada >> num_nos;
    for(int i=1 ; i<=num_nos*n_col_nos; i++){
        if(i % n_col_nos == 1){
            entrada >> idNo;
//            cout << "idNo: " << idNo;
        }
        else if(i % n_col_nos == 4){
            entrada >> voltagem;
//            cout << "      V: " << voltagem;
        }
        else if(i % n_col_nos == 10){
            entrada >> carga;
            carga /= PB;
            carga *=fator_MW;
//            cout << "      pot at=carga: " << carga;
        }
        else if(i % n_col_nos == 11){
            entrada >> potencia_reativa;
            potencia_reativa /=PB;
            potencia_reativa *= fator_MW;
//            cout << "      pot reat: " << potencia_reativa << endl;
            insereNo(idNo, carga, potencia_reativa, voltagem);
        }
        else
            entrada >> aux;
    }

    do{
        entrada >> aux;
    }while(aux != "num_arestas");///pula mais lixo ate chegar na parte de informacoes da aresta

    int num_arestas;
    entrada >> num_arestas;

    for(int i=1 ; i<=num_arestas*n_col_arestas; i++){
        if(i % n_col_arestas == 1){
            entrada >> idOrig;
//            cout << "idOrig: " << idOrig;
        }
        else if(i % n_col_arestas == 2){
            entrada >> idDest;
//            cout << "      idDest: " << idDest;
        }
        else if(i % n_col_arestas == 3){
            entrada >> resistencia;
            resistencia /= ZB;
//            cout << "      r: " << resistencia;
        }
        else if(i % n_col_arestas == 4){
            entrada >> reatancia;
            reatancia /= ZB;
//            cout << "      reat: " << reatancia;
            //insere arco i-j e j-i

            idArco++;
            insereArco(idOrig, idDest, idArco, resistencia, reatancia, false);
            insereArco(idDest, idOrig, idArco, resistencia, reatancia, false);
//            cout << "  inseriu arco!" << endl;
        }
        else
            entrada >> aux;
    }

//    cout << "\n" << nome << " lida!" << endl;
}

void Graph_network::imprime(){
    printf("\n\n\nGRAFO  num_Nos: %d    num_arcos: %d \n\n\n", this->numeroNos, this->numeroArcos);
    for(Vertex *no = this->listaNos; no != NULL; no = no->getProxNo())
        no->imprime();
}

double Graph_network::cargasPerdasRamoAtiv(Vertex *no){
    double soma =no->getPotAtiva();
    auxcargasPerdasRamoAtiv(no, soma);
    return soma;
}

void Graph_network::auxcargasPerdasRamoAtiv(Vertex *no, double &soma){
    for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){

        //nao descer por arcos com chave aberta
        while(a!=NULL && a->getChave() == false){

            //nao tem fluxo nem perda em arcos abertos
            a->setFLuxoPAtiva(0.0);
            a->setFLuxoPReativa(0.0);
            a->setPerdaAtiva(0.0);
            a->setPerdaReativa(0.0);

            a = a->getProxArco();
        }
        if(a==NULL)
            break;

        soma+=a->getNoDestino()->getPotAtiva() + a->getPerdaAtiva();
        auxcargasPerdasRamoAtiv(a->getNoDestino(), soma);
    }
}

double Graph_network::cargasPerdasRamoReAtiv(Vertex *no){
    double soma=no->getPotReativa();
    auxcargasPerdasRamoReAtiv(no, soma);
    return soma;
}

void Graph_network::auxcargasPerdasRamoReAtiv(Vertex *no, double &soma){
    for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){

        //nao descer por arcos com chave aberta
        while(a!=NULL && a->getChave() == false){

            //nao tem fluxo nem perda em arcos abertos
            a->setFLuxoPAtiva(0.0);
            a->setFLuxoPReativa(0.0);
            a->setPerdaAtiva(0.0);
            a->setPerdaReativa(0.0);

            a = a->getProxArco();
        }
        if(a==NULL)
            break;

        soma+=a->getNoDestino()->getPotReativa() + a->getPerdaReativa();
        auxcargasPerdasRamoReAtiv(a->getNoDestino(), soma);
    }
}

//FOWARD
void Graph_network::foward(int it){
    this->listaNos->setVoltagem(1.0);//voltagem controlada na estacao
    Auxfoward(this->listaNos, this->listaNos->getListaArcos(), it);
}

void Graph_network::Auxfoward(Vertex *no, Edge *ak, int it){
    if(no == NULL)
        cout<<"\n No NULL \n"<<endl;
    else{
        for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){

            double perda_ativ = 0.0;
            double perda_reat = 0.0;

            //nao descer por arcos com chave aberta
            while(a!=NULL && a->getChave() == false){

                //nao tem fluxo nem perda em arcos abertos
                a->setFLuxoPAtiva(0.0);
                a->setFLuxoPReativa(0.0);
                a->setPerdaAtiva(0.0);
                a->setPerdaReativa(0.0);

                a = a->getProxArco();
            }
            if(a==NULL)
                break;

            //----foward----

            //chute inicial para o fluxo nas arests que partem do no terminal
            if(no==this->listaNos){
                double carcasPerdasAtivRamo = cargasPerdasRamoAtiv(a->getNoDestino());
                double carcasPerdasReativRamo = cargasPerdasRamoReAtiv(a->getNoDestino());

                a->setFLuxoPAtiva(carcasPerdasAtivRamo + a->getPerdaAtiva());
                a->setFLuxoPReativa(carcasPerdasReativRamo + a->getPerdaReativa());

            }else{

                //a partir da primeira iteracao considerase a perda calculada na iteracao anterior
                if(it>0){
                    perda_ativ = ak->getResistencia()*(pow(ak->getFLuxoPAtiva(), 2) + pow(ak->getFLuxoReativa(), 2)) / pow(ak->getNoOrigem()->getVoltagem(), 2);
                    perda_reat = ak->getReatancia()*(pow(ak->getFLuxoPAtiva(), 2) + pow(ak->getFLuxoReativa(), 2)) / pow(ak->getNoOrigem()->getVoltagem(), 2);
                }

                a->setFLuxoPAtiva(ak->getFLuxoPAtiva() - perda_ativ - no->getPotAtiva());
                a->setFLuxoPReativa(ak->getFLuxoReativa() - perda_reat - no->getPotReativa());

                //bifurcacao - o fluxo que seque para o arco 'a' deve-se subtrair a soma de cargas e perdas dos ramos de bifurcacoes
                if(a->getNoOrigem()->getGrauSaida()>1){
                    double somaAtiv=0.0, somaReAtiv=0.0;
                    for(Edge *aux=no->getListaArcos(); aux != NULL; aux=aux->getProxArco()){
                        if(aux!=a && aux->getChave()==true){
                            somaAtiv+=aux->getPerdaAtiva() + cargasPerdasRamoAtiv(aux->getNoDestino());
                            somaReAtiv+=aux->getPerdaReativa() + cargasPerdasRamoReAtiv(aux->getNoDestino());
                        }
                    }

                    a->setFLuxoPAtiva(a->getFLuxoPAtiva() - somaAtiv);
                    a->setFLuxoPReativa(a->getFLuxoReativa() - somaReAtiv);
                }
            }
            //--------------

            Auxfoward(a->getNoDestino(), a, it);
        }
    }
}

void Graph_network::backward(){
    this->listaNos->setVoltagem(1.0);//voltagem controlada na estacao
    Auxbackward(this->listaNos);
}

void Graph_network::Auxbackward(Vertex *no){
    if(no == NULL)
        cout<<"\n No NULL \n"<<endl;
    else{
        for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){

            //nao descer por arcos com chave aberta
            while(a!=NULL && a->getChave() == false){

                //nao tem fluxo nem perda em arcos abertos
                a->setFLuxoPAtiva(0.0);
                a->setFLuxoPReativa(0.0);
                a->setPerdaAtiva(0.0);
                a->setPerdaReativa(0.0);

                a = a->getProxArco();
            }
            if(a==NULL)
                break;

            //----backward----
            Vertex *noDest = a->getNoDestino();
            Vertex *noOrig = a->getNoOrigem();

            noDest->setVoltagem(pow(noOrig->getVoltagem(), 2)
            - 2*(a->getResistencia()*a->getFLuxoPAtiva() + a->getReatancia()*a->getFLuxoReativa()) +
            (pow(a->getResistencia(), 2) + pow(a->getReatancia(), 2))*
            (pow(a->getFLuxoPAtiva(), 2) + pow(a->getFLuxoReativa(), 2))/pow(noOrig->getVoltagem(), 2));

            noDest->setVoltagem(sqrt(noDest->getVoltagem()));

            a->setPerdaAtiva(a->getResistencia()*(pow(a->getFLuxoPAtiva(), 2) + pow(a->getFLuxoReativa(), 2))
            /pow(noOrig->getVoltagem(), 2));
            a->setPerdaReativa(a->getReatancia()*(pow(a->getFLuxoPAtiva(), 2) + pow(a->getFLuxoReativa(), 2))
            /pow(noOrig->getVoltagem(), 2));
            //-------------

            Auxbackward(a->getNoDestino());
        }
    }
}

double *Graph_network::soma_perdas(){
    double *perda = new double[2]; perda[0] = perda[1] = 0.0;
    for(Vertex *no=listaNos; no != NULL; no = no->getProxNo()){
        for(Edge *a = no->getListaArcos(); a != NULL; a = a->getProxArco()){
            perda[0] += a->getPerdaAtiva();
            perda[1] += a->getPerdaReativa();
        }
    }
    return perda;
}

double *Graph_network::soma_perdasResetando(){
    double *perda = new double[2]; perda[0] = perda[1] = 0.0;
    for(Vertex *no=listaNos; no != NULL; no = no->getProxNo()){
        no->setIdArv(no->getID());
        for(Edge *a = no->getListaArcos(); a != NULL; a = a->getProxArco()){
            perda[0] += a->getPerdaAtiva();
            perda[1] += a->getPerdaReativa();

            //arcos nao modificaveis ficam sempre fechados
            if(a->getModificavel()==false)
                a->setChave(true);
            else
                a->setChave(false);

            a->setFLuxoPAtiva(0.0);
            a->setFLuxoPReativa(0.0);
            a->setPerdaAtiva(0.0);
            a->setPerdaReativa(0.0);
        }
    }
    return perda;
}

void Graph_network::calcula_fluxos_e_perdas(double tol){

    double *perdas_total_anterior, *perdas_total_atual, erro = 1.0;

    //processo iterativo até convergir com erro=tol
    for(int it=0; erro>tol; it++){

        perdas_total_anterior = this->soma_perdas();

        //calcula fluxos de potencia nas linhas
        foward(it);

        //calcula voltagem nas barras e perdas nas linhas
        backward();

        perdas_total_atual = this->soma_perdas();
        erro = perdas_total_atual[0] - perdas_total_anterior[0];

//        cout << "\n\nperda total da rede: (" << perdas_total_atual[0] << " , " << perdas_total_atual[1] << ")" << endl;
//        cout << "DIFERENCA ENTRE PERDA ATUAL E ANTERIOR CALCULADA: ( " << perdas_total_atual[0] - perdas_total_anterior[0]
//        << " , " << perdas_total_atual[1] - perdas_total_anterior[1] << " )\n";

        delete perdas_total_anterior;
        delete perdas_total_atual;

    }

//    printf("fluxos e perdas calculados!\n");
}

void Graph_network::desmarcaNos(){
    for(Vertex *i = this->listaNos; i != NULL; i = i->getProxNo())
        i->setMarcado(false);
    this->n_marcados = 0;
}

bool Graph_network::ehConexo(){
    this->desmarcaNos();
    int n_marcados = 0;
    auxehConexo(this->listaNos, n_marcados);

    if(n_marcados == this->numeroNos)
        return true;
    else
        return false;
}

void Graph_network::auxehConexo(Vertex *no, int &n_marcados){
    if(no == NULL)
        cout<<"\n No NULL \n"<<endl;
    else{
        if(no->getMarcado() == false){
            no->setMarcado(true);
            n_marcados++;

            for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){

                //nao descer por arcos com chave aberta
                while(a!=NULL && a->getChave() == false){
                    a = a->getProxArco();
                }

                if(a==NULL)
                    break;

                auxehConexo(a->getNoDestino(), n_marcados);
            }
        }
    }
}

bool Graph_network::ehArvore(){
    desmarcaNos();

    bool ciclo = false;
    int marcados = 0;

    auxehArvore(listaNos, marcados, ciclo);

    if(ciclo==true)
        cout << "\nFECHOU CICLO!!" << endl;
    if(marcados<numeroNos)
        cout << "\nARVORE NAO COBRE TODOS OS NOS!!";

    if(ciclo==false && marcados==numeroNos)
        return true;
    else
        return false;
}

void Graph_network::auxehArvore(Vertex *no, int &marcados, bool &ciclo){
    if(no == NULL)
        cout<<"\n No NULL \n"<<endl;
    else{
        if(no->getMarcado() == false){
            no->setMarcado(true);
            marcados++;

            for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){

                //nao descer por arcos com chave aberta
                while(a!=NULL && a->getChave() == false){
                    a = a->getProxArco();
                }

                if(a==NULL)
                    break;

                auxehArvore(a->getNoDestino(), marcados, ciclo);
            }
        }
        else
            ciclo = true;
    }
}

double Graph_network::tensaoMinima(){
    double tensao_mim = 1.0;
    for(Vertex *no=this->listaNos; no != NULL; no=no->getProxNo()){
        if(no->getVoltagem() < tensao_mim)
            tensao_mim = no->getVoltagem();
    }
    return tensao_mim;
}

void Graph_network::define_sentido_fluxos(){
    auxDefine_sentido_fluxos(this->listaNos, this->listaNos);
//    printf("\nsentido de fluxo das arestas definido!\n");
}

void Graph_network::auxDefine_sentido_fluxos(Vertex *no, Vertex *noAnterior){
    if(no == NULL)
        cout<<"\n No NULL \n"<<endl;
    else{
        for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){

            //nao descer por arcos com chave aberta e descer no sentido correto
            //a->getNoDestino()==no e o arco de volta do arco 'a' assim nao passamos por ele
            while(a!=NULL && (a->getChave() == false || a->getNoDestino()==noAnterior) ){

                //nao tem fluxo nem perda em arcos abertos
                a->setFLuxoPAtiva(0.0);
                a->setFLuxoPReativa(0.0);
                a->setPerdaAtiva(0.0);
                a->setPerdaReativa(0.0);

                if(a->getNoDestino()==noAnterior)
                    a->setChave(false);

                a = a->getProxArco();
            }
            if(a==NULL)
                break;

//            printf("\nno{%d}", no->getID());
            auxDefine_sentido_fluxos(a->getNoDestino(), no);
        }
    }
}

void Graph_network::defineArestasModificaveis(){

    this->resetaModificaveis();//define todos os arcos como sendo modificaveis
    int n = 0;
    for(int i=0; i<this->numeroArcos/2; i++){


        //procura arcos que conectam nos folha, esses arcos nao sao modificaveis(nao podem ser abertos)
        for(Vertex *no=this->listaNos; no != NULL; no=no->getProxNo()){
            for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){

                if(a->getNoDestino()->getGrauAux()==1 && a->getChave()==true && a->getModificavel()==true){

//                    printf("\ndefinindo como nao modif A{%d}...", a->getID());

                    a->setModificavel(false);
                    a->setChave(true);//nao modificavel: chave sempre fechad

                    Edge *aVolta = this->buscaArco(a->getNoDestino()->getID(), a->getNoOrigem()->getID());
                    aVolta->setModificavel(false);

                    a->getNoOrigem()->setGrauAux(a->getNoOrigem()->getGrauAux()-1);
                    a->getNoDestino()->setGrauAux(a->getNoDestino()->getGrauAux()-1);

                    //arcos nao modificaveis ficam sempre fechados


//                    printf("done!");

                    n++;
                }
            }
        }
    }
    this->n_naoModificaveis = n;
//    printf("\ndefiniu arestas modificaveis!");
}

Graph_network *Graph_network::retornaCopia(){
    Graph_network *g = new Graph_network();
    int idOrig, idDest, idArco, idNo;
    double resistencia, reatancia, potAtiva, potReativa, voltagem;

    for(Vertex *no=this->listaNos->getProxNo(); no != NULL; no=no->getProxNo()){
        idNo = no->getID();
        potAtiva = no->getPotAtiva();
        potReativa = no->getPotReativa();
        voltagem = no->getVoltagem();

        g->insereNo(idNo, potAtiva, potReativa, voltagem);
    }

    //inserindo no fonte por ultimo pos a funcao que insere nos insere sempre no inicio da lista, assim o no fonte fica no inicio da lista
    Vertex *noFonte = this->listaNos;
    g->insereNo(noFonte->getID(), noFonte->getPotAtiva(), noFonte->getPotReativa(), noFonte->getVoltagem());

    for(Vertex *no=this->listaNos; no != NULL; no=no->getProxNo()){
         for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){
            idOrig = a->getNoOrigem()->getID();
            idDest = a->getNoDestino()->getID();
            idArco = a->getID();
            resistencia = a->getResistencia();
            reatancia = a->getReatancia();

            g->insereArco(idOrig, idDest, idArco, resistencia, reatancia, a->getChave());
        }
    }

    return g;
}

/*Algoritmo de Kruskal randomizado*/
void Graph_network::solucaoAleatoria(){
    vector<Edge*> vetArcos;
    for(Vertex *no = this->getListaNos(); no != NULL; no = no->getProxNo()){
        for(Edge *a = no->getListaArcos(); a != NULL; a = a->getProxArco()){
            a->setChave(false);
            vetArcos.push_back(a);
        }
    }

    random_shuffle(vetArcos.begin(), vetArcos.end());

    int n_arc_inseridos = 0, n_arcos_inserir = this->numeroNos-1;
    for(int i=0; n_arc_inseridos<n_arcos_inserir; i++){

        if( (vetArcos.at(i)->getNoOrigem()->getIdArv() != vetArcos.at(i)->getNoDestino()->getIdArv()) && vetArcos.at(i)->getChave()==false){

            int id = vetArcos.at(i)->getNoOrigem()->getIdArv();
            for(Vertex *no = this->listaNos; no != NULL; no = no->getProxNo()){
                if(no->getIdArv()==id)
                    no->setIdArv(vetArcos.at(i)->getNoDestino()->getIdArv());
            }

            vetArcos.at(i)->setChave(true);
            this->buscaArco(vetArcos.at(i)->getNoDestino()->getID(), vetArcos.at(i)->getNoOrigem()->getID())->setChave(true);

            n_arc_inseridos++;
        }

    }

    printf("  Aberto:{");
    for(Vertex *no = this->listaNos; no != NULL; no = no->getProxNo()){
        for(Edge *a = no->getListaArcos(); a != NULL; a = a->getProxArco()){
            if(a->getChave()==false)
                printf("%d,", a->getID());
        }
    }
    printf("  }");
}

void Graph_network::resetaGrausAuxiliares(){
    for(Vertex *no = this->listaNos; no != NULL; no = no->getProxNo()){
        for(Edge *a = no->getListaArcos(); a != NULL; a = a->getProxArco()){
            Vertex *orig, *dest;
            orig = a->getNoOrigem();
            dest = a->getNoDestino();

            orig->setGrauAux(orig->getGrauEntrada());
            dest->setGrauAux(dest->getGrauEntrada());
        }
    }
}

void Graph_network::resetaIdArv(){
    for(Vertex *no = this->listaNos; no != NULL; no = no->getProxNo()){
        no->setIdArv(no->getID());
    }
}

void Graph_network::zeraFluxosEPerdas(){
    for(Vertex *no=this->listaNos; no != NULL; no=no->getProxNo()){
        for(Edge *a=no->getListaArcos(); a != NULL; a=a->getProxArco()){
            a->setFLuxoPAtiva(0.0);
            a->setFLuxoPReativa(0.0);
            a->setPerdaAtiva(0.0);
            a->setPerdaReativa(0.0);
        }
    }
}

void Graph_network::fechaArcosNaoModificaveis(){
    for(Vertex *no = this->listaNos; no != NULL; no = no->getProxNo()){

        for(Edge *a = no->getListaArcos(); a != NULL; a = a->getProxArco()){
            if(a->getModificavel() == false){
                a->setChave(true);
            }
        }

    }
}

void Graph_network::resetaModificaveis(){
    for(Vertex *no = this->listaNos; no != NULL; no = no->getProxNo()){

        for(Edge *a = no->getListaArcos(); a != NULL; a = a->getProxArco()){
            a->setModificavel(true);
        }
    }
}

void Graph_network::resetaArcosMarcados(){
    for(Vertex *no = this->listaNos; no != NULL; no = no->getProxNo()){

        for(Edge *a = no->getListaArcos(); a != NULL; a = a->getProxArco()){
            a->setMarcado(false);
        }
    }
}

/** percorre todos os arcos do grafo e marca somente os arcos em um sentido
(funcao necessaria para otimizar o AG utilizando metade do numero de arcos) **/
void Graph_network::marcaUmsentidoArcos(){
    this->resetaArcosMarcados();
    Edge *aux;
    for(Vertex *no = this->listaNos; no != NULL; no = no->getProxNo()){

        for(Edge *a = no->getListaArcos(); a != NULL; a = a->getProxArco()){

            if(a->getMarcado()==false){
                a->setMarcado(false);
                aux = this->buscaArco(a->getNoDestino()->getID(), a->getNoOrigem()->getID());
                aux->setMarcado(true);
            }

        }
    }
}

void Graph_network::marcaComponenteConexa(Vertex *start, int value) {
    aux_marcaComponenteConexa(start, start, value);
}

void Graph_network::aux_marcaComponenteConexa(Vertex *start, Vertex *previous, int value) {
    if(start == NULL)
        cout<<"\n No NULL \n"<<endl;
    else{
        for(Edge *a=start->getListaArcos(); a != NULL; a=a->getProxArco()){

            if(a->getChave()==true) {
                this->buscaArco(a->getNoDestino()->getID(), a->getNoOrigem()->getID())->setChave(false);
//                start->setIdArv(value);
                a->getNoOrigem()->setIdArv(value);
                a->getNoDestino()->setIdArv(value);
                this->aux_marcaComponenteConexa(a->getNoDestino(), start, value);
            }
        }
    }
}

void Graph_network::percursoPronfundidade(Vertex *start, int value) {
    if(start==NULL)
        printf("inicio NULL\n");
    else{
        for(Edge *e = start->getListaArcos(); e!=NULL; e = e->getProxArco()){
            start->setIdArv(value);
            printf("no: %d\t", start->getID());
            this->percursoPronfundidade(e->getNoDestino(), value);
        }
    }
}