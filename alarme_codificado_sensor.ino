#include <Keypad.h> // inclui a biblioteca do teclado matricial 4x4
#include <LiquidCrystal.h> // inclui a biblioteca para o display Lcd 16x2

#define led_status A0 // define pino do LED ON/OFF
#define buzzer A1 // define pino do BUZZER
#define sinal_sensor A2 // defino pino que recebe sinal do sensor

byte pinosLinhas[4] = {7, 6, 5, 4}; //define pinos das linhas do teclado 8x8
byte pinosColunas[3] = {3, 2, A5}; //define pinos das colunas do teclado 8x8
//MATRIZ DO TECLADO
char teclas[4][3] = {{'1','2','3'}, 
                     {'4','5','6'},
                     {'7','8','9'},
                     {'*','0','#'}};


int ativa_alarme; // Variavel ativa alarme usada em loop
bool monitoramento; //variavel para loop de monitoramento
bool disparo_alarme; // variavel para ligar buzzer
bool ativado; // variavel para loop de ativacao ou tentativa de ativacao do monitoramento

int tentativa; //variavel que armazena tentativas de digitar a senha (nao sao em todos cenarios)

int leitura_sensor; // variavel para receber leitura do sensor

char senhaAtivar[4] = {'7','3','9','1'}; // vetor que armazena a senha para ATIVAR o sensor 
char senhaDesativar[4] = {'5','9','3','8'}; // vetor que armazena a senha para desativar o sensor 
char vetor[4]; // vetor para armazenar linhas do teclado
int i; // variavel de incremento de index

int tempo_bloqueio = 5000; //tempo de bloqueio depois que erra a senha

//INICIALIZACAO DO TECLADO E DO DISPLAY
Keypad teclado1 = Keypad( makeKeymap(teclas), pinosLinhas, pinosColunas, 4, 3 );
LiquidCrystal lcd(8,9,10,11,12,13); //D7,D6,D4,E,RS


void setup() {
    pinMode(buzzer, OUTPUT); // Buzzer como saida
    pinMode(led_status, OUTPUT); // define o pino do led como saída
    pinMode(sinal_sensor,INPUT); // define sensor como recebedor de sinal

    //INICIALIZACAO COM DISPLAY
    lcd.begin(16,2);
    lcd.setCursor(0,0); 
    lcd.print("ALARME 2021");
    delay(1000);
    limparTodoDisplay();
    lcd.setCursor(0,0); 
    lcd.print("INICIALIZANDO...");
    delay(1000);
    limparTodoDisplay(); //limpa todo display

    monitoramento = false; //monitoramento como false para inicio

    digitalWrite (led_status ,LOW); // inicia LED como desligado
    digitalWrite (buzzer, LOW); //inicia buzzer como desligado


    tentativa = 3; //definie primeira tentativa como 3

} 



void loop() {

    lcd.setCursor(0,0); 
    lcd.print("SENSOR DESATIV.");

    lcd.setCursor(0,1); 
    lcd.print("ATIVAR NO '#'    ");

    digitalWrite(led_status, LOW);    
   
    char tecla_pressionada = teclado1.getKey(); 

    if (tecla_pressionada == '#') { // verificar se # foi pressionado
        monitoramento = false;
        ativado = true;
        lcd.clear();
        
        tecla_pressionada = teclado1.getKey(); // pega tecla do teclado 
        
        while ( ativa_alarme == 0 && ativado == true){ // entra no loop para esperar senha         
            digitalWrite(led_status, LOW);
            
            if(tentativa == 0){ //caso acabe as tentativas, sai do loop e volta tela
                lcd.clear();
                ativado = false;
            }
            
            lcd.setCursor(0,0);
            lcd.print("SENHA ATIVAR:"); 
            inserirSenhaExibir();   

            tecla_pressionada = teclado1.getKey(); // verifica se alguma tecla foi pressionada
            if (tecla_pressionada){  // se alguma tecla for pressionada, executa abaixo
                vetor[i] = tecla_pressionada; // armazena tecla no vetor

                if(vetor[i] == '*'){ //caso foi pressionado *, sai da digitaçao
                    ativado = false;
                    lcd.clear();
                } else if (tecla_pressionada >= '0' && tecla_pressionada <= '9') {
                    lcd.setCursor(i+3 ,1); //corre o display
                    lcd.print('X'); // mostra tecla * para nao mostrar a senha
                    
                    i++; // contador para definir que a senha seja de 4 digitos

                    if(i == 4){ // se o ultimo digito da senha for pressionado, executa abaixo
                        if(vetor[0] == senhaAtivar[0] && vetor[1] == senhaAtivar[1] 
                        && vetor[2] == senhaAtivar[2] && vetor[3] == senhaAtivar[3]){ 
                            // compara o vetor das teclas pressionadas com a senha pré definida

                            senhaCorretaAviso();
                            delay(1000);
                            limparLinhaUmDisplay();

                            statusOnOffExibir(true);
                            delay(1000); 

                            i=0; // zera o contador
    
                            ativa_alarme = 1;
                            monitoramento = true; 
                            ativado = true;

                            limparTodoDisplay();

                            tentativa = 3; //reseta tentativas para novo loop
                            bool cancelar = true; // para caso apertar cancelar, nao travar

                            while (ativa_alarme == 1 && monitoramento) { //loop de monitoramento do sensor
                                lcd.setCursor(0,0); 
                                lcd.print("MONITORANDO..."); 

                                //Logica para trava do sistema ao errar senhas
                                if(tentativa == 0 && cancelar == false){ //Caso acabe as tentativas, fica travado por tempo determinado
                                    limparLinhaDoisDisplay();
                                    
                                    lcd.setCursor(0,1);
                                    lcd.print("TRAVADO-AGUARDE");
                                    delay(tempo_bloqueio);    //tempo travado
                                    limparLinhaDoisDisplay();
                                    tentativa = 3;
                                }
                                
                                lcd.setCursor(0,1);
                                lcd.print("DESATIVAR NO '#'");

                                digitalWrite(led_status, HIGH); //acende o LED pois monitoramento foi ligado

                                tecla_pressionada = teclado1.getKey(); // verifica se alguma tecla foi pressionada                                
                                if(tecla_pressionada == '#'){ //tecla para desativar for pressionada
                                    tentativa = 3;
                                    limparLinhaDoisDisplay();

                                    while(tentativa > 0){       
                                        leitura_sensor = digitalRead(sinal_sensor); // leitura do sinal do sensor
                                        /* 
                                        ESSA LEITURA É FEITA POIS O SENSOR PODE APITAR ENQUANTO ESTÁ SENDO DIGITADA A SENHA
                                        PARA DESATIVAR O SENSOR. CASO SENSOR VIBRAR, TROCARÁ O ALARME, SAIRÁ DESSE LOOP ZERANDO AS TENTATIVAS
                                        */

                                        if(leitura_sensor) tentativa = 0;

                                        inserirSenhaExibir();
                                        tentativasExibir(tentativa);
                                
                                        tecla_pressionada = teclado1.getKey(); 
                                        if (tecla_pressionada){  
                                            vetor[i] = tecla_pressionada; // armazena no vetor[4] as teclas pressionadas
                                            
                                            if(vetor[i] == '*'){
                                                cancelar = true;
                                                limparLinhaDoisDisplay();
                                                tentativa = 0;
                                            // } else {
                                            } else if (tecla_pressionada >= '0' && tecla_pressionada <= '9') {
                                                lcd.setCursor(i+3,1); 
                                                // lcd.print(tecla_pressionada); // indica no display a tecla pressionada
                                                lcd.print('X');
                                                i++; // contador armazenar a tecla e percorrer a escrita do LCD
                                                if(i==4){ 
                                                    if(vetor[0] == senhaDesativar[0] && 
                                                        vetor[1] == senhaDesativar[1] &&
                                                        vetor[2] == senhaDesativar[2] &&
                                                        vetor[3] == senhaDesativar[3]){ 

                                                        limparTodoDisplay();

                                                        senhaCorretaAviso();
                                                        delay(300);
                                                        limparLinhaUmDisplay();

                                                        statusOnOffExibir(false);

                                                        //desativa o buzzar as 2 linhas abaixo
                                                        analogWrite(buzzer, 0); 
                                                        digitalWrite(buzzer, LOW); 
                                                        delay(1000); 

                                                        i=0; // zera o contador

                                                        ativa_alarme = 0;
                                                        monitoramento = false;
                                                        ativado = false;

                                                        limparTodoDisplay();

                                                        break;

                                                    } 
                                                    else { // se a senha não for correta, coloca executa abaixo aviso                                       
                                                        senhaInvalidaAviso();
                                                        i = 0; //volta o index a 0 para escrita no começo
                                                        cancelar = false;
                                                        tentativa--; //decrementa uma tentativa
                                                    }
                                                }
                                            }   
                                        }                         
                                    }
                                }


                                leitura_sensor = digitalRead(sinal_sensor); // realiza a leitura de sinal advindo do sensor de vibrações
                                if (leitura_sensor == HIGH) { // se receber sinal do sensor...
                                    limparTodoDisplay();
                                    lcd.setCursor(0,0);
                                    lcd.print("VIBRANDO!!!");

                                    disparo_alarme = true; //para o loop ficar disparando buzzer

                                    while (disparo_alarme){
                                        inserirSenhaExibir();  // SÓ PARA DE DISPARAR QUANDO COLOCAR A SENHA     
                                        
                                        tecla_pressionada = teclado1.getKey();                

                                        //Disparo do buzzer e pisca o LED
                                        piscarLED();
                                        alarmeSonoroAtivar();
                                       
                                        tecla_pressionada = teclado1.getKey(); // verifica se alguma tecla foi pressionada
                                        if (tecla_pressionada >= '0' && tecla_pressionada <= '9') {
                                            vetor[i] = tecla_pressionada; 
                                            lcd.setCursor(i+3,1); 
                                            lcd.print('X');
                                            i++; 
                                            if(i==4){ 
                                                if(vetor[0] == senhaDesativar[0] && 
                                                    vetor[1] == senhaDesativar[1] &&
                                                    vetor[2] == senhaDesativar[2] &&
                                                    vetor[3] == senhaDesativar[3]){ 

                                                    limparTodoDisplay();
                                                    senhaCorretaAviso();

                                                    statusOnOffExibir(false);

                                                    analogWrite(buzzer, 0); 
                                                    digitalWrite(buzzer, LOW); 
                                                    delay(1000); // indica que a senha digitada foi correta

                                                    i=0; // zera o index

                                                    disparo_alarme = false; //desabilita o disparo saindo do loop

                                                    break;

                                                } 
                                                else {                                     
                                                    senhaInvalidaAviso();
                                                    i = 0;
                                                }
                                            } 
                                        }
                                    }
                                    // FIM do alarme
                                } else {
                                    analogWrite(buzzer,0); // caso o alarme não estiver ativo, garante o buzzer desativado
                                }
                            }

                        } else { // se a senha PARA ATIVACAO não for correta, executa abaixo                    
                            senhaInvalidaAviso();
                            i = 0;
                        } 
                    }                

                }

            }
        }
    }
}

//para limpar todo o display
/* 
HOUVE PROBLEMAS COM LCD.CLEAR()
ENTAO, FOI FEITO PARA LIMPAR UMA LINHA POR VEZ E UM METODO
QUE CHAME OS DOIS 
*/
void limparTodoDisplay(){
    limparLinhaUmDisplay();
    limparLinhaDoisDisplay();
}

void limparLinhaUmDisplay(){
    lcd.setCursor(0,0); 
    lcd.print("                "); // limpa o display
}
void limparLinhaDoisDisplay(){
    lcd.setCursor(0,1); 
    lcd.print("                "); // limpa o display
}

void senhaInvalidaAviso(){
    limparLinhaDoisDisplay();
    lcd.setCursor(0,1);
    lcd.print("SENHA INCORRETA");
    delay(700);
    limparLinhaDoisDisplay();
}

void senhaCorretaAviso(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SENHA CORRETA");
    lcd.setCursor(0,1);
    lcd.print("AGUARDE...");
    delay(100);
    limparLinhaUmDisplay();
    delay(300);
    limparLinhaDoisDisplay();

}

void statusOnOffExibir(bool status){
    limparTodoDisplay();
    lcd.setCursor(0,0);

    if(status){
        lcd.print("ATIVANDO...");
    } else {
        lcd.print("DESATIVANDO...");
    }

    delay(1000);
}

//INDICACAO DE COLOCAR SENHA... HÁ TELAS QUE NÃO HOUVE ESPAÇO
void inserirSenhaExibir(){
    lcd.setCursor(0,1); 
    lcd.print("->");
}

void tentativasExibir(int tentativa){
    lcd.setCursor(13,1);
    lcd.print("(");
    lcd.setCursor(14,1);
    lcd.print(tentativa);
    lcd.setCursor(15,1);
    lcd.print(")");
}

void piscarLED(){
    digitalWrite(led_status, HIGH);
    delay(70);
    digitalWrite(led_status, LOW);
}

void alarmeSonoroAtivar(){
    digitalWrite (buzzer, HIGH); 
    delay(50); // Alarme 
    analogWrite (buzzer, 150); 
    delay(100); // sonorização alarme inicial
    digitalWrite (buzzer, LOW); 
}