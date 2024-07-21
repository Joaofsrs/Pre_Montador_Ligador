#include <iostream>
#include <bits/stdc++.h>
#include <map>
#include <stdio.h>
#include <string.h>
using namespace std;

map<string, pair<int, int>> opcode{
    {"ADD", make_pair(1, 2)},
    {"SUB", make_pair(2, 2)},
    {"MULT", make_pair(3, 2)},
    {"MUL", make_pair(3, 2)},
    {"DIV", make_pair(4, 2)},
    {"JMP", make_pair(5, 2)},
    {"JMPN", make_pair(6, 2)},
    {"JMPP", make_pair(7, 2)},
    {"JMPZ", make_pair(8, 2)},
    {"COPY", make_pair(9, 3)},
    {"LOAD", make_pair(10, 2)},
    {"STORE", make_pair(11, 2)}, 
    {"INPUT", make_pair(12, 2)},
    {"OUTPUT", make_pair(13, 2)},
    {"STOP", make_pair(14, 1)},
    {"SPACE", make_pair(0, 0)},
    {"CONST", make_pair(0, 0)},
};

string erros = "";
vector<pair<string, int>> tabela_simbolo;
vector<vector<string>> codigo_assembly;
vector<string> label_name;
map<string, int> tabela_simbolos;
vector<int> contador;
vector<vector<int>> codiu_final;
string objeto = "", nome_prog;
bool e = 0;
map<string, string> dicionario_EQU;
map<string, string> cause5hora;

/*
    Funcao responsavel por gerar o arquivo, sempre eh a ultima funcao chamada, a primeira string que le recebe contem o nome do arquivo, a segunda eh o
    tipo(.PRE e .OBJ) e o ultimo argumento eh a string com o programa completo
*/
void cria_arquivo(string nome, string formato, string texto)
{
    FILE *arq;
    char num[80], text[1000];
    string path = nome + formato;
    strcpy(num, path.c_str());
    arq = fopen(num, "w+");

    if (arq == NULL)
    {
        cout << "Erro ao criar arquivo" << endl;
        return;
    }

    strcpy(text, texto.c_str());

    if (fputs(text, arq) == EOF)
        cout << "Erro na Gravacao" << endl;

    fclose(arq);
}

// Função auxiliar para converter string para maiúsculas
string to_upper(string &str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

// Função auxilia para retirar dois pontos
string label_sem_dois_pontos(string label){
    string label_formatada;

    for(int i = 0; i < label.length(); i ++){
        if(label[i] != ':'){
            label_formatada = label_formatada + label[i];
        }
    }

    return label_formatada;
}

// Função para verificar se uma string e 0, 00, 000, etc
bool verifica_zero(string num){
    for(int i = 0; i < num.length(); i++){
        if(num[i] != '0'){
            return false;
        }
    }
    return true;
}

// verifica se uma linha realmente e vazia
bool verifica_vazio(string linha){
    for(int i = 0; i < linha.size(); i++){
        if(isalnum(linha[i]) || linha[i] == '_' || linha[i] == ':' || linha[i] == '+' || linha[i] == ','){
            return false;
        }
    }
    return true;
}

// Função principal de processamento
void pre_processamento(FILE *program) {
    // algumas bandeiras para identificar if, equ, quebra de linha, linha depois do if
    bool band = true, linha_if = false, quebra_linha = false, linha_equ = false, not_if = false, tem_mais = false, teve_mais = false; 
    // vetor que quardas os otkens de uma linha 
    vector<string> linha_comando;
    // map que salva os valores de um equ usando seu nome como chave exemplo
    // AA: EQU 1 map - equ['AA'] = 1
    map<string, string> equ;

    while(band){
        // variaveis para ler linha por linha do programa .asm
        char linha[100];
        string result = "";

        // adiciona na variavel result a linha do programa caso ainda exista, caso contrario para a execusao do loop
        if (fgets(linha, sizeof(linha), program) != nullptr) {
            result = linha;
        } else {
            band = false;
            break;
        }

        // variavel token que salva temporariamente o token enquanto le as linhas
        string token;
        string last_token;
        // caso o if esteja como IF 0 ele ignora a proxima linha e verifica no else se essa linha não e vazia
        if(!not_if){
            // for usado para andar pelas linhas e identificar token por token para verificar se estão certos
            for (int i = 0; i < result.size(); ++i) {
                // usado para para em caso de comentario
                if (result[i] == ';') break;
                // caso seja um EQU ele marca a bandeira para salvar na tabela mais adiante
                if(to_upper(token) == "EQU"){
                    linha_equ = true;
                // caso seja um IF ele marca a bandeira para identificar mais para frente se esta como 0 ou 1
                }else if(to_upper(token) == "IF"){
                    linha_if = true;
                }
                // vai salvando letra por letra no token caso seja valido
                if(result[i] == '+'){
                    tem_mais = true;
                    if(!token.empty()){
                        token += result[i];
                    }else{
                        teve_mais = true;
                        token = last_token + result[i];
                    }
                }else if(isalnum(result[i]) || result[i] == '_' || result[i] == ':' || result[i] == ','){
                    token += result[i];
                    tem_mais = false;
                // para o caso de espaco entra para salvar o token no vetor que salva o programa 
                }else if(isspace(result[i]) && !tem_mais){   
                    // caso seja vazio ele nao salva
                    if(!token.empty()){
                        // caso o token exista na tabela de EQU ele troca o nome pelo valor do EQU, caso contrario apenas coloca o token na lista
                        if(teve_mais && !linha_comando.empty()){
                            teve_mais = false;
                            linha_comando.pop_back();
                        }
                        if(equ.find(token) != equ.end()){
                            linha_comando.push_back(to_upper(equ.at(token)));
                            last_token = token;
                            token.clear();
                            quebra_linha = false;
                        }else{
                            linha_comando.push_back(to_upper(token));
                            last_token = token;
                            token.clear();
                            quebra_linha = false;
                        }
                    }
                }
            }
        }else{
            // em caso de ser uma linha apos uma linha ignorada apos um if vai pulara ate que seja um linha nao vazia para ignora-la
            if (!verifica_vazio(result)) {
                not_if = false;
            }
        }

        // insere o ultimo token no vetor pois o for nao insere ele no final, mas antes verifica se nao e vazio
        if (!token.empty()) {
            if(equ.find(token) != equ.end()){
                linha_comando.push_back(to_upper(equ.at(token)));
                token.clear();
                quebra_linha = false;
            }else{
                linha_comando.push_back(to_upper(token));
                token.clear();
                quebra_linha = false;
            }
        }

        // utilizando a bandeira de marcador caso tenha identificado um EQU nessa linha
        // pega a linha e adiciona no map de EQU o valor tendo label como chave
        if(linha_equ){
            string aux_label = "";
            if(!linha_comando[0].empty()){
                aux_label = label_sem_dois_pontos(linha_comando[0]);
            }   
            if(!linha_comando[linha_comando.size()-1].empty()){
                equ.insert(make_pair(aux_label, linha_comando[linha_comando.size()-1]));
            }
        }
        
        // utilizando a bandeira marcada como linha de IF entra para verificar se o IF
        // vai como 0 ou 1
        if(linha_if){
            if(!linha_comando[1].empty()){
                if(to_upper(linha_comando[0]) == "IF"){
                    if(!linha_comando[1].empty()){
                        // verifica para o caso de ser um 0 ele coloca a bandeira not_if para um IF FALSE
                        // para que ele ignore a proxima linha
                        if(verifica_zero(linha_comando[1])){
                            not_if = true;
                        }
                    }
                }else if (to_upper(linha_comando[1]) == "IF"){
                    if(!linha_comando[2].empty()){
                        // verifica para o caso de ser um 0 ele coloca a bandeira not_if para um IF FALSE
                        // para que ele ignore a proxima linha
                        if(verifica_zero(linha_comando[2])){
                            not_if = true;
                        }
                    }
                }
            }

        }


        // verifica se existe uma linha para adiciona a tabela geral 'codigo_assembly' que e um vector<vector<string>>
        if(linha_comando.size() > 0){
            if(!linha_comando[0].empty()){
                // para o caso de ser uma label e em seguida uma quebra de linha ele aciona a bandeira quebra_linha
                // assim ele segue para a proxima linha sem para tirar essa quebra de linha no processo
                if(linha_comando[0][linha_comando[0].size() - 1] == ':' && linha_comando.size() == 1){
                    quebra_linha = true;
                }
            }
            // caso nao seja uma quebra de linha ou IF e EQU ele ignora e nao adiciona já que sao linhas que devem ser excluidas
            if(!quebra_linha && !linha_if && !linha_equ){
                codigo_assembly.push_back(linha_comando);
            }
        }

        // caso nao seja uma quebra de linha ele zera as bandeiras e variaveis segue para a proxima linhas
        // caso contrario ele segue preenchendo o vetor de linha
        if(!quebra_linha){
            linha_if = false;
            linha_equ = false;
            linha_comando.clear();
        }
    }
    // apos todo o programa ja pre processado estar salvo na variavel global 'codigo_assembly'
    // o arquivo e fechado 
    fclose(program);
}

// usada para passar toda a variavel 'codigo_assembly' do tipo vector<vector<string>> para uma unica string
void cria_arq_pre()
{
    string novo_arq;
    for (int i = 0; i < codigo_assembly.size(); i++)
    {
        for (int j = 0; j < codigo_assembly[i].size(); j++)
        {
            novo_arq += string(codigo_assembly[i][j]) + " ";
        }
        novo_arq += "\n";
    }
    novo_arq.pop_back();
    // passa o nome do arquivo, extencao e o conteudo(em formato de string)
    cria_arquivo(nome_prog, ".pre", novo_arq);
}

// apenas formata para ficar 00, 01, 02 no lugar de 0, 1, 2, etc.
string formata_numero(int number) {
    ostringstream numero_com_zero;
    numero_com_zero << setw(2) << setfill('0') << number;
    return numero_com_zero.str();
}


// usado para formatar para uma string o arquivo obj da passagem unica simples
void passa_arq_obj(vector<int> porg_obj)
{
    string novo_arq;
    for (int i = 0; i < porg_obj.size(); i++)
    {
        novo_arq += formata_numero(porg_obj[i]) + " ";
    }
    novo_arq.pop_back();
    cria_arquivo(nome_prog, ".obj", novo_arq);
}

// usado para verificar se tem um + em uma variavel
pair<string, int> tem_mais(string palavra)
{
    string saida = "";
    string numeroS = "0";
    int numero = 0;
    int mais = 0;
    for (int i = 0; i < palavra.size(); i++)
    {
        if (palavra[i] != '+' && mais == 0)
        {
            saida += palavra[i];
        }
        else if (palavra[i] == '+')
        {
            mais = 1;
        }
        else if (mais == 1 && (palavra[i] >= '0' && palavra[i] <= '9'))
        {
            numeroS += palavra[i];
        }
    }
    numero = stoi(numeroS);
    return make_pair(saida, numero);
}

// Funcao usada para converter as variaveis do copy que vem separadas por virgula
pair<pair<string, int>, pair<string, int>> acha_copy(string valores_copy) {
    string var1 =  "", var2 = "";
    bool band_vigula = false;

    for(int i = 0; i < valores_copy.length(); i++){
        if(valores_copy[i] == ','){
            band_vigula = true;
        }else if(band_vigula){
            var2 = var2 + valores_copy[i];
        }else if(!band_vigula){
            var1 = var1 + valores_copy[i];
        }
    }

    pair<string, int> par1 = tem_mais(var1);
    pair<string, int> par2 = tem_mais(var2);;

    return make_pair(par1, par2);
}

// Funcao usada para verificar se uma label e valida ou nao, para o caso de inicar com um nome que nao pode
bool verifica_label_valida(string label){
    for(int i = 0; i < label.length()-1; i++){
        if((i == 0 && label[i] >= '0' && label[i] <= '9') || (!isalnum(label[i]) && label[i] != '_') || (opcode.find(label_sem_dois_pontos(label)) != opcode.end())){
            return false;
        }
    }
    return true;
}

/* 
    Algoritmo de passagem unica, le e adiciona automaticamente ao vertor final_exc 
*/
void passagem_unica()
{
    int k = 0, comando = 0, flag_label = 0, erro = 0;   // bandeiras usadas no decorrer do codigo
    string erros = "";  // varivel de erros para criar o arquivo de erros ao final caso exista
    int endereco_atual = 0, linha_atual = 0;    // variaveis usadas para se localizar mostarando a linha e o endereco atual
    vector<int> final_exc;  // armazena o codigo objeto final
    map<string, vector<int>> lista_simbolo;  // guarda a lista de simbolos/pendencias 
    for (int i = 0; i < codigo_assembly.size(); i++)
    {
        flag_label = 0;
        int cont_space = 0, const_band = 0, stop = 0, copy = 0;
        for (int j = 0; j < codigo_assembly[i].size(); j++)
        {   
            // Entra no if para o caso de edentifcar um label
            if (codigo_assembly[i][j][codigo_assembly[i][j].size() - 1] == ':')
            {
                flag_label = 1;
                // caso seja uma label valida armazena na tabela de simbolos e na tabela auxiliar com os nomes das labels
                if (codigo_assembly[i][j] != "" && verifica_label_valida(codigo_assembly[i][j]))
                {
                    label_name.push_back(codigo_assembly[i][j].substr(0, codigo_assembly[i][j].size() - 1));
                    tabela_simbolos[codigo_assembly[i][j].substr(0, codigo_assembly[i][j].size() - 1)] = linha_atual;
                }else if(!verifica_label_valida(codigo_assembly[i][j])){
                    erro = 1;
                    erros += "ERRO LEXICO DE ROTULO NO ROTULO:  " + codigo_assembly[i][j] +  " \n";
                    cout << "ERRO LEXICO DE ROTULO NO ROTULO:  " << codigo_assembly[i][j] << endl;
                }
            }
            // caso nao seja um label e considerado um comando
            else if (comando == 0)
            {   
                // verifica se e um comando valido
                if(opcode.find(codigo_assembly[i][j]) != opcode.end()){
                    comando = 1;
                }else{
                    erro = 1;
                    erros += "ERRO SINTATICO INSTRUCAO OU DIRETIVA INVALIDA \n";
                    cout << "ERRO SINTATICO INSTRUCAO OU DIRETIVA INVALIDA" << endl;
                }

                // para o caso de se um copy 
                if (codigo_assembly[i][j] == "COPY")
                {
                    pair<int, int> soma = opcode.find(codigo_assembly[i][j])->second;
                    final_exc.push_back(soma.first);        // adiciona o opcode do copy na lista do codigo objeto
                    endereco_atual++;   // endereco atual + 1
                    // verifica para o caso de existir um duplicata de copy
                    if(codigo_assembly[i][codigo_assembly[i].size() -1] != "COPY"){

                        // tem como retorno duas variaveis pair que seriam as variaveis do copy separadas por virgula
                        pair<pair<string, int>, pair<string, int>> resultado = acha_copy(codigo_assembly[i][codigo_assembly[i].size() -1]);

                        // adicona na lista de pendencia/simbolos o endereco da variavel
                        pair<string, int> saida1 = resultado.first;
                        lista_simbolo[saida1.first].push_back(endereco_atual);
                        endereco_atual++;
                        // adicona na lista de pendencia/simbolos o endereco da variavel
                        pair<string, int> saida2 = resultado.second;
                        lista_simbolo[saida2.first].push_back(endereco_atual);
                        endereco_atual++;
                        // adiciona no o valor 0 para o caso de uma varivel comum ou o valor da soma do endereco para o caso de um endereco 
                        // ex.: ENDER+2 adiciona dois na lista de objeto
                        final_exc.push_back(saida1.second);
                        final_exc.push_back(saida2.second);
                        copy = 1;
                        j = codigo_assembly[i].size();
                    }else{
                        if(codigo_assembly[i].size()-1 > j){
                            erro = 1;
                            erros += "ERRO LEXICO USO DE NOME RESTRITO \n";
                            cout << "ERRO LEXICO USO DE NOME RESTRITO" << endl;
                        }else{
                            erro = 1;
                            erros += "ERRO SINTATICO FALTA DE TOKEN(VARIAVEL NO COPY) \n";
                            cout << "ERRO SINTATICO FALTA DE TOKEN(VARIAVEL NO COPY)" << endl;
                        }
                    }
                }

                // para o caso de stop
                else if (codigo_assembly[i][j] == "STOP")
                {   
                    // adicona o opcode na lista objeto
                    pair<int, int> soma = opcode.find(codigo_assembly[i][j])->second;
                    final_exc.push_back(soma.first);
                    endereco_atual++;
                    stop = 1;
                    // caso tenha algum operado no stop e considerado um erro
                    if(codigo_assembly[i].size()-1 > j){
                        erros += "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO \n";
                        cout << "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO" << endl;
                    }
                    j = codigo_assembly[i].size();
                }

                // para o caso de space
                else if (codigo_assembly[i][j] == "SPACE")
                {
                    // verifica caso o space tenha mais de um operando 
                    if((codigo_assembly[i].size()-1 > 3 && flag_label) || (codigo_assembly[i].size()-1 > 2 && !flag_label)){
                        erros += "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO \n";
                        cout << "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO" << endl;
                    }
                    // para o caso de ter algum operando ele verifica a quantidade de enderecos que devem ser reservados e adiciona a lista objeto
                    else if (codigo_assembly[i].size() - 1 > j)
                    {
                        for (int k = 0; k < stoi(codigo_assembly[i][j + 1]); k++)
                        {
                            final_exc.push_back(0);
                            endereco_atual++;
                            cont_space++;
                        }
                    }
                    // caso nao tenha reserva um espaco apenas e adiciona a lista objeto
                    else
                    {
                        final_exc.push_back(0);
                        endereco_atual++;
                        cont_space++;
                    }
                    j = codigo_assembly[i].size();
                }
                // para o caso de const
                else if (codigo_assembly[i][j] == "CONST")
                {
                    // verifica se possui o numero correto de operandos
                    if(codigo_assembly[i].size()-1 != j){
                        // verifcia se esta em hexadecimal ou nao e adiciona na lista objeto
                        if(codigo_assembly[i][j+1][0] == '0' && codigo_assembly[i][j+1][1]== 'X'){
                            // converte para int e adiciona na lista objeto
                            int num = stoi(codigo_assembly[i][j+1], nullptr, 16);
                            final_exc.push_back(num);
                            endereco_atual++;
                            const_band = 1;
                            j = codigo_assembly[i].size();
                        }else{
                            // insere na lista objeto
                            final_exc.push_back(stoi(codigo_assembly[i][j + 1]));
                            endereco_atual++;
                            const_band = 1;
                            j = codigo_assembly[i].size();
                        }
                    }else{
                        erros += "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO \n";
                        cout << "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO" << endl;
                    }
                }
                // para o caso de ser um comando nao tratado anteriormente
                else
                {   
                    // todos os comandos que entram aqui sao esperados apenas o comando e o operando
                    if((codigo_assembly[i].size() != 3 && flag_label) || (codigo_assembly[i].size() != 2 && !flag_label)){
                        erros += "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO \n";
                        cout << "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO" << endl;
                    }else{
                        // adiciona o opcode na lista objeto
                        pair<int, int> soma = opcode.find(codigo_assembly[i][j])->second;
                        final_exc.push_back(soma.first);
                        endereco_atual++;
                        // verifica a existencia de um um mais no endereco enviado
                        pair<string, int> saida = tem_mais(codigo_assembly[i][j + 1]);
                        // caso ja tenha sido criado ele apenas pega o valor caso contrario adiciona na lista de pendencias
                        if (tabela_simbolos.find(saida.first) == tabela_simbolos.end())
                        {
                            lista_simbolo[saida.first].push_back(endereco_atual);
                            final_exc.push_back(saida.second);
                        }
                        else
                        {
                            final_exc.push_back(tabela_simbolos.find(saida.first)->second + saida.second);
                        }
                        endereco_atual++;
                    }
                    
                    j = codigo_assembly[i].size();
                }
            }
        }
        // pula a linha de acordo com o comando chamado na linha anterior 
        if (codigo_assembly[i].size() > 0)
        {
            if (copy == 1)
            {
                linha_atual += 3;
            }
            else if (stop == 1 || const_band == 1)
            {
                linha_atual += 1;
            }
            else if (cont_space != 0)
            {
                linha_atual += cont_space;
            }
            else
            {
                linha_atual += 2;
            }
        }
        stop = 0;
        copy = 0;
        const_band = 0;
        cont_space = 0;
        comando = 0;
    }
    // resolve a lista de pendencias
    // andndo pelas labels ja crias
    for (int i = 0; i < label_name.size(); i++)
    {
        map<string, vector<int>>::iterator teste = lista_simbolo.find(label_name[i]);
        // verifica a existencia de uma lista de pendencias
        if (teste != lista_simbolo.end())
        {   
            // verifica a lista e vai direto ao enreco indicado na lista e substitui ele
            vector<int> saida = lista_simbolo.find(label_name[i])->second;
            for (int j = 0; j < saida.size(); j++)
            {
                if (tabela_simbolos.find(label_name[i]) != tabela_simbolos.end())
                {
                    // faz uma operacao de soma pos caso o endereco seja X+2 o 2 esta salvo na lista objeto
                    final_exc[saida[j]] = final_exc[saida[j]] + tabela_simbolos.find(label_name[i])->second;
                }
            }
        }
    }
    // caso tenha erros ele cria um arquivo com os erros e nao o arquivo com o codigo objeto
    if(erro){
        string nome_arquvio_erros = "erros" + nome_prog;
        cria_arquivo(nome_arquvio_erros, ".txt", erros);
    }else{
        passa_arq_obj(final_exc);
    }
}


// verifica a existencia de uma string em um vertor de strings
bool existe_string(vector<string> vetor, string name){
    for(int i = 0; i < vetor.size(); i++){
        if(vetor[i] == name){
            return true;
        }
    }
    return false;
}

// apenas pega o nome do arquivo(usado apenas na main)
string nome_arquivo(string name_with_dot){
    string name_aux;
    for(int i = 0; i < name_with_dot.length(); i++){
        if(name_with_dot[i] == '.'){
            break;
        }else{
            name_aux = name_aux + name_with_dot[i];
        }
    }
    return name_aux;
}

// converte as tableas em string para que o arquivo seja .obj seja criado
void arq(vector<pair<string, int>> tabela_uso, vector<pair<string, int>> tabela_definicao, vector<int> relativos, vector<int> final_exc){
    string saida;
    saida += "USO\n";
    for (int i = 0; i < tabela_uso.size(); i++)
    {
        saida += tabela_uso[i].first + " " + to_string(tabela_uso[i].second) + "\n";
    }
    saida += "DEF\n";
    for (int i = 0; i < tabela_definicao.size(); i++)
    {
        saida += tabela_definicao[i].first + " " + to_string(tabela_definicao[i].second) + "\n";
    }
    saida += "\nREAL\n";
    for (int i = 0; i < relativos.size(); i++)
    {
        saida += to_string(relativos[i]);
    }
    saida += "\n\n";
    for (int i = 0; i < final_exc.size(); i++)
    {   
        saida += to_string(final_exc[i]) + " ";
    }
    cria_arquivo(nome_prog, ".obj", saida);
}

// Essa fincao e para o quando possui begin/end no codigo
void passagem_unica_ligador()
{
    int k = 0, comando = 0, flag_label = 0, begin_band = 0, erro = 0;   // bandeiras utilizadas no decorrer do codigo
    int endereco_atual = 0, linha_atual = 0;    // usado para slavar a linha e o endereco no momento
    vector<int> final_exc;  // usado para salvar o codigo objeto
    map<string, vector<int>> lista_simbolo; // faz a funcao da lista de pendencias
    vector<string> label_name_extern;  // salva o nome das labels externas 
    vector<string> label_name_public;  // salva o nome dasa labels publicas
    vector<pair<string, int>> tabela_uso; // salva a tabela de uso 
    vector<pair<string, int>> tabela_definicao; // salva a tabela de definicao
    vector<int> relativo_absoluto; // salva o que e relativo e absoluto 
    pair<string, int> begin_label; // salva a label inical
    string erros; // salva os erros identificados

    // anda pelo codigo assembly enviado
    for (int i = 0; i < codigo_assembly.size(); i++)
    {
        flag_label = 0;
        int cont_space = 0, const_band = 0, stop = 0, copy = 0;
        // anda pela linha do cosigo
        for (int j = 0; j < codigo_assembly[i].size(); j++)
        {   
            // caso identifique um label
            if (codigo_assembly[i][j][codigo_assembly[i][j].size() - 1] == ':')
            {
                flag_label = 1;
                // verifica se e uma label valida
                if (codigo_assembly[i][j] != "" && verifica_label_valida(codigo_assembly[i][j]))
                {
                    if (codigo_assembly[i].size()-1 > j)
                    {   
                        // caso seja uma label verifica a possibilidade de um extern 
                        if(to_upper(codigo_assembly[i][j+1]) == "EXTERN"){
                            // adiciona o nome na lista de label externas
                            string label_name_var = codigo_assembly[i][j].substr(0, codigo_assembly[i][j].size() - 1);
                            label_name_extern.push_back(to_upper(label_name_var));
                            j = codigo_assembly[i].size();
                        }
                        // caso seja uma label verifica a possibilidade de um begin 
                        else if(to_upper(codigo_assembly[i][j+1]) == "BEGIN"){
                            // adiciona la variavel begin_label para salvar essa label pois ela e importante
                            begin_band = 1;
                            string label_name_var = codigo_assembly[i][j].substr(0, codigo_assembly[i][j].size() - 1);
                            begin_label = make_pair(to_upper(label_name_var), 0);
                            j = codigo_assembly[i].size();
                        }
                        // caso seja uma label comum apenas salva na lista de pendencias
                        else{
                            label_name.push_back(codigo_assembly[i][j].substr(0, codigo_assembly[i][j].size() - 1));
                            tabela_simbolos[codigo_assembly[i][j].substr(0, codigo_assembly[i][j].size() - 1)] = endereco_atual;
                        }
                    }
                    // no caso da label criada estar como publica verifica para colocar na lista de definicao
                    string label_name_definicao = codigo_assembly[i][0].substr(0, codigo_assembly[i][0].size() - 1);
                    if(existe_string(label_name_public, label_name_definicao)){
                        pair<string, int> label_pair = make_pair(to_upper(label_name_definicao), endereco_atual);
                        tabela_definicao.push_back(label_pair);
                    }
                }else if(!verifica_label_valida(codigo_assembly[i][j])){
                    erro = 1;
                    erros += "ERRO LEXICO DE ROTULO NO ROTULO:  " + codigo_assembly[i][j] +  " \n";
                    cout << "ERRO LEXICO DE ROTULO NO ROTULO:  " << codigo_assembly[i][j] << endl;
                }
                
            }
            // para o caso de um public
            else if(codigo_assembly[i][j] == "PUBLIC")
            {   
                // verifica para o caso de nao ter o numero de operandos
                if (codigo_assembly[i].size()-1 > j)
                {   
                    // verifica se nao e a label inicial 
                    if(codigo_assembly[i][j+1] == begin_label.first){
                        // no caso de ser a label inicial adiciona direto na tabela de definicao
                        tabela_definicao.push_back(begin_label);
                    }else{
                        // adiciona na lista de labels publicas para verificar posteriormente
                        label_name_public.push_back(codigo_assembly[i][j+1]);
                    }
                    j = codigo_assembly[i].size();
                }else{
                    erro = 1;
                    erros += "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO \n";
                    cout << "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO" << endl;
                }
            }
            // para o caso de end
            else if(codigo_assembly[i][j] == "END"){
                // verifica se possui begin pois caso nao retorna erro
                if(begin_band){
                    break;
                }else{
                    erro = 1;
                    erros += "ERRO SEMANTIGO END SEM BEGIN \n";
                    cout << "ERRO SEMANTIGO END SEM BEGIN" << endl;
                }
            }
            // caso seja um comando
            else if (comando == 0)
            {
                // verifica se e um comando valido
                if(opcode.find(codigo_assembly[i][j]) != opcode.end()){
                    comando = 1;
                }else{
                    erro = 1;
                    erros += "ERRO SINTATICO INSTRUCAO OU DIRETIVA INVALIDA \n";
                    cout << "ERRO SINTATICO INSTRUCAO OU DIRETIVA INVALIDA" << endl;
                }
                comando = 1;

                // verifica para o caso do copy
                if (codigo_assembly[i][j] == "COPY")
                {   
                    // adiciona o opcode na lista objeto
                    pair<int, int> soma = opcode.find(codigo_assembly[i][j])->second;
                    final_exc.push_back(soma.first);
                    endereco_atual++;
                    // adiciona na lista de relativos e absolutos
                    relativo_absoluto.push_back(0);
                    
                    if(codigo_assembly[i][codigo_assembly[i].size() -1] != "COPY"){
                        relativo_absoluto.push_back(1);
                        relativo_absoluto.push_back(1);
                        // retorna os dois operandos do copy sem a virgula
                        pair<pair<string, int>, pair<string, int>> resultado = acha_copy(codigo_assembly[i][codigo_assembly[i].size() -1]);
                        pair<string, int> saida1 = resultado.first;
                        
                        // verifiaca se a varival usada e externa
                        if(existe_string(label_name_extern, saida1.first)){
                            // caso seja externa adiciona na tabela de uso 
                            pair<string, int> par_uso;
                            par_uso = make_pair(saida1.first, endereco_atual);
                            tabela_uso.push_back(par_uso);
                        }
                        // caso contrario apenas adiciona na lista de pendencias
                        else{
                            lista_simbolo[saida1.first].push_back(endereco_atual);
                        }
                        endereco_atual++;
                    
                        pair<string, int> saida2 = resultado.second;
                        // verifiaca se a varival usada e externa
                        if(existe_string(label_name_extern, saida2.first)){
                            // caso seja externa adiciona na tabela de uso 
                            pair<string, int> par_uso;
                            par_uso = make_pair(saida2.first, endereco_atual);
                            tabela_uso.push_back(par_uso);
                        }
                        // caso contrario apenas adiciona na lista de pendencias
                        else{
                            lista_simbolo[saida2.first].push_back(endereco_atual);
                        }
                        endereco_atual++;
                        // adiciona na lista objeto
                        final_exc.push_back(saida1.second); 
                        final_exc.push_back(saida2.second);
                        copy = 1;
                        j = codigo_assembly[i].size();
                    }else{
                        // caso tenha mais de um significa que e uma duplicata ou uso de variavel restrita como endereco
                        if(codigo_assembly[i].size()-1 > j){
                            erro = 1;
                            erros += "ERRO LEXICO USO DE NOME RESTRITO \n";
                            cout << "ERRO LEXICO USO DE NOME RESTRITO" << endl;
                        }else{
                            // caso contrario tem uma falta de operandos
                            erro = 1;
                            erros += "ERRO SINTATICO FALTA DE TOKEN(VARIAVEL NO COPY) \n";
                            cout << "ERRO SINTATICO FALTA DE TOKEN(VARIAVEL NO COPY)" << endl;
                        }
                    }
                }
                // para o caso de um stop
                else if (codigo_assembly[i][j] == "STOP")
                {
                    // adicona o opcode na lista objeto e na lista de relativos e absolutos
                    relativo_absoluto.push_back(0);
                    pair<int, int> soma = opcode.find(codigo_assembly[i][j])->second;
                    final_exc.push_back(soma.first);
                    endereco_atual++;
                    stop = 1;
                    // caso tenha algum operado no stop e considerado um erro
                    if(codigo_assembly[i].size()-1 > j){
                        erros += "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO \n";
                        cout << "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO" << endl;
                    }
                    j = codigo_assembly[i].size();
                }
                else if (codigo_assembly[i][j] == "SPACE")
                {
                    // verifica caso o space tenha mais de um operando 
                    if((codigo_assembly[i].size()-1 > 3 && flag_label) || (codigo_assembly[i].size()-1 > 2 && !flag_label)){
                        erros += "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO \n";
                        cout << "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO" << endl;
                    }
                    // para o caso de ter algum operando ele verifica a quantidade de enderecos que devem ser reservados e adiciona a lista objeto
                    else if (codigo_assembly[i].size() - 1 > j)
                    {
                        for (int k = 0; k < stoi(codigo_assembly[i][j + 1]); k++)
                        {
                            relativo_absoluto.push_back(0);
                            final_exc.push_back(0);
                            endereco_atual++;
                            cont_space++;
                        }
                    }
                    // caso nao tenha reserva um espaco apenas e adiciona a lista objeto
                    else
                    {
                        relativo_absoluto.push_back(0);
                        final_exc.push_back(0);
                        endereco_atual++;
                        cont_space++;
                    }
                    j = codigo_assembly[i].size();
                }
                else if (codigo_assembly[i][j] == "CONST")
                {
                    // verifica se possui o numero correto de operandos
                    if(codigo_assembly[i].size()-1 != j){
                        // verifcia se esta em hexadecimal ou nao e adiciona na lista objeto
                        if(codigo_assembly[i][j+1][0] == '0' && codigo_assembly[i][j+1][1]== 'X'){
                            // converte para int e adiciona na lista objeto
                            int num = stoi(codigo_assembly[i][j+1], nullptr, 16);
                            final_exc.push_back(num);
                            endereco_atual++;
                            relativo_absoluto.push_back(0);
                            const_band = 1;
                            j = codigo_assembly[i].size();
                        }else{
                            // adiciona na lista objeto
                            final_exc.push_back(stoi(codigo_assembly[i][j + 1]));
                            endereco_atual++;
                            relativo_absoluto.push_back(0);
                            const_band = 1;
                            j = codigo_assembly[i].size();
                        }
                    }else{
                        erros += "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO \n";
                        cout << "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO" << endl;
                    }
                }
                // para o caso de um comando que nao entrou anteriormente
                else
                {   
                    // todos os comandos que entram aqui sao esperados apenas o comando e o operando
                    if((codigo_assembly[i].size() != 3 && flag_label) || (codigo_assembly[i].size() != 2 && !flag_label)){
                        erros += "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO \n";
                        cout << "ERRO SINTATICO NUMERO DE OPERANDOS ERRADOS PARA INSTRUCAO" << endl;
                    }else{                        
                        // adiciona o opcode na lista objeto e o valor de absoluto e relativo
                        relativo_absoluto.push_back(0);
                        pair<int, int> soma = opcode.find(codigo_assembly[i][j])->second;
                        final_exc.push_back(soma.first);
                        endereco_atual++;
                        // verifica a existencia de um um mais no endereco enviado
                        pair<string, int> saida = tem_mais(codigo_assembly[i][j + 1]);
                        // caso ja tenha sido criado ele apenas pega o valor caso contrario adiciona na lista de pendencias
                        if (tabela_simbolos.find(saida.first) == tabela_simbolos.end())
                        {   
                            // verifica se a label usada e externa ou nao
                            if(existe_string(label_name_extern, saida.first)){
                                // caso seja externa adiciona na tabela de uso 
                                pair<string, int> par_uso;
                                par_uso = make_pair(saida.first, endereco_atual);
                                tabela_uso.push_back(par_uso);
                            }
                            // caso contrario apenas adiciona na lista de pendencias
                            else{
                                lista_simbolo[saida.first].push_back(endereco_atual);
                            }
                            final_exc.push_back(saida.second);
                            relativo_absoluto.push_back(1);
                            endereco_atual++;
                        }
                        else
                        {   
                            // verifica se a label usada e externa ou nao
                            if(existe_string(label_name_extern, saida.first)){
                                // caso seja externa adiciona na tabela de uso 
                                pair<string, int> par_uso;
                                par_uso = make_pair(saida.first, endereco_atual);
                                tabela_uso.push_back(par_uso);
                            }
                            // caso contrario apenas adiciona na lista de pendencias
                            else{
                                final_exc.push_back(tabela_simbolos.find(saida.first)->second + saida.second);
                            }
                            // adicoina na lista de relativos
                            relativo_absoluto.push_back(1);
                            endereco_atual++;
                        }
                    }
                    j = codigo_assembly[i].size();
                }
            }
        }
        // faz a contagem das linhas de acordo com a operacao anterior
        if (codigo_assembly[i].size() > 0)
        {
            if (copy == 1)
            {
                linha_atual += 3;
            }
            else if (stop == 1 || const_band == 1)
            {
                linha_atual += 1;
            }
            else if (cont_space != 0)
            {
                linha_atual += cont_space;
            }
            else if(comando)
            {
                linha_atual += 2;
            }
        } 
        stop = 0;
        copy = 0;
        const_band = 0;
        cont_space = 0;
        comando = 0;
    }
    // resolve a listad e pendencias
    // andando pela lista de labels criadas
    for (int i = 0; i < label_name.size(); i++)
    {
        map<string, vector<int>>::iterator teste = lista_simbolo.find(label_name[i]);
        // verifica se a label em questao possui uma lista de pendencias
        if (teste != lista_simbolo.end())
        {
            vector<int> saida = lista_simbolo.find(label_name[i])->second;
            // resolve a lista e pendencias
            for (int j = 0; j < saida.size(); j++)
            {
                if (tabela_simbolos.find(label_name[i]) != tabela_simbolos.end())
                {
                    // soma o endereco com o que esta no local da lista objeto pois pode ser que seja chamada com soma
                    // ex.: X+2
                    final_exc[saida[j]] = final_exc[saida[j]] + tabela_simbolos.find(label_name[i])->second;
                }
            }
        }
    }
    // caso tenha erros ele cria um arquivo com os erros e nao o arquivo com o codigo objeto
    if(erro){
        string nome_arquvio_erros = "erros" + nome_prog;
        cria_arquivo(nome_arquvio_erros, ".txt", erros);
    }else{
        arq(tabela_uso, tabela_definicao, relativo_absoluto, final_exc);
    }
}


/*
    Apenas faz a leitura e passa para um variavel 'codigo_assembly' no formato vector<vector<string>>
    nao analiza nada ja que e um arquivo pre processado, apenas pega os as informaçoes e usa o espaco para separar como um item do vetor
*/
void normalizacao_encaminhamento(FILE *program){
    bool band = true;
    vector<string> linha_comando;
    while(band){
        // variaveis para ler linha por linha do programa .pre
        char linha[100];
        string result = "";

        // adiciona na variavel result a linha do programa caso ainda exista, caso contrario para a execusao do loop
        if (fgets(linha, sizeof(linha), program) != nullptr) {
            result = linha;
        } else {
            band = false;
            break;
        }

        // variavel token que salva temporariamente o token enquanto le as linhas
        string token;

        for (int i = 0; i < result.size(); ++i) {
            // vai salvando letra por letra no token caso ache um espaco ou quebra de linha ele entra no else if
            if(!isspace(result[i])){
                token += result[i];
            // para o caso de espaco entra para salvar o token no vetor que salva o programa 
            }else if(isspace(result[i])){   
                // caso seja vazio ele nao salva
                if(!token.empty()){
                    // vai adicionando token por token para salvar na variavel 
                    linha_comando.push_back(to_upper(token));
                    token.clear();
                }
            }
        }
        // caso seja vazio ele nao salva, esse if e para o caso de um \n
        if(!token.empty()){
            // vai adicionando token por token para salvar na variavel 
            linha_comando.push_back(to_upper(token));
            token.clear();
        }
        // salva a linha na variavel 'codigo_assembly'
        codigo_assembly.push_back(linha_comando);
        linha_comando.clear();
    }
    // o arquivo e fechado 
    fclose(program);

    if(codigo_assembly.size() > 0){
        if(codigo_assembly[0].size() > 1){
            if(to_upper(codigo_assembly[0][1]) == "BEGIN"){
                passagem_unica_ligador();
            }else{
                passagem_unica();
            }
        }else{
            passagem_unica();
        }
    }else{
        passagem_unica();
    }
}

/*
    A main faz o tratamento da entrada do programa, escolhendo as funcoes para rodar e o tipo de arquivo a ser pego
    caso aconteca algum erro em -o uma flag eh acionada e ao ter o return na funcao e voltar para a main se a flag eh 0
    o programa encerra salvando o Log.txt
*/
int main(int argc, char *argv[])
{
    FILE *program;
    char num[80];
    string tipo = argv[1];

    // O tipo -p é usado quando o programa quer apenas o arquivo .asm pré processado 
    if(tipo == "-p"){
        nome_prog = nome_arquivo(argv[2]); // salva o nome do programa para salvar o arquivo com o mesmo nome
        string path = string(argv[2]);  // pegar o nome do arquivo que deve ser aberto
        strcpy(num, path.c_str());  
        program = fopen(num, "rb"); // abre o arquivo salvo na pasta
        pre_processamento(program); // inicia o pre processamento
        cria_arq_pre(); // cria o arquivo pre pre processado
    }
    // O tipo -o é usado quando o programa quer apenas o arquivo objeto do aquivo .pre 
    if(tipo == "-o"){
        nome_prog = nome_arquivo(argv[2]); // salva o nome do programa para salvar o arquivo com o mesmo nome
        string path = string(argv[2]);  // pegar o nome do arquivo que deve ser aberto
        strcpy(num, path.c_str());  
        program = fopen(num, "rb"); // abre o arquivo salvo na pasta
        normalizacao_encaminhamento(program); // apenas normaliza não chega a ser um pre processamento
    }
}