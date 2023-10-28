#include <WiFi.h>  
#include <PubSubClient.h>
#include <DHTesp.h>

const int DHT_PIN = 15;  // Pino onde o sensor DHT22 está conectado
DHTesp dht;  // Objeto para interagir com o sensor DHT22
const char* ssid = "Wokwi-GUEST"; // SSID da rede Wi-Fi
const char* password = ""; // Senha da rede Wi-Fi
const char* mqtt_server = "test.mosquitto.org"; // URL do servidor MQTT

WiFiClient espClient; // Cria um objeto WiFiClient chamado espClient para gerenciar a conexão Wi-Fi
PubSubClient client(espClient); // Cria um objeto PubSubClient chamado client para trabalhar com MQTT usando espClient como cliente de rede MQTT
unsigned long lastMsg = 0; // Variável para rastrear o tempo da última publicação de dados MQTT
float temp = 0; // Variável para armazenar a temperatura lida do sensor DHT22
float hum = 0; // Variável para armazenar a umidade lida do sensor DHT22

void setup_wifi() { 
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid); // Exibe o SSID da rede Wi-Fi no monitor serial

  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password); // Inicia a conexão Wi-Fi com as credenciais especificadas

  while (WiFi.status() != WL_CONNECTED) { 
    // Enquanto o status da conexão Wi-Fi não estiver como "WL_CONNECTED" (ou seja, não estiver conectado),
    // o programa continuará neste loop aguardando a conexão ser estabelecida.
    // Isso é necessário para garantir que o dispositivo ESP32 esteja conectado à rede Wi-Fi antes de prosseguir.
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected"); // Exibe a mensagem quando a conexão Wi-Fi é estabelecida

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // Exibe o endereço IP local atribuído ao dispositivo
}

void reconnect() { 
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection..."); // Tentando estabelecer a conexão MQTT
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX); // Gera um ID de cliente MQTT aleatório

    if (client.connect(clientId.c_str())) {
      Serial.println("Connected"); // Exibe a mensagem quando a conexão MQTT é bem-sucedida
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state()); // Exibe o estado de conexão MQTT
      Serial.println(" try again in 5 seconds"); // Exibe uma mensagem de falha e tenta novamente após 5 segundos
      delay(5000);
    }}
}

void setup() {
  randomSeed(micros()); // Inicializa o gerador de números aleatórios com uma semente baseada no tempo atual (microssegundos)
  Serial.begin(115200); // Inicializa a comunicação serial com uma taxa de baud de 115200
  setup_wifi(); // Chama a função para configurar a conexão Wi-Fi
  client.setServer(mqtt_server, 1883); // Configura o servidor MQTT e a porta
  dht.setup(DHT_PIN, DHTesp::DHT22); // Inicializa o objeto DHTesp para ler dados do sensor DHT22
}

void loop() {
  if (!client.connected()) {
    reconnect(); // Se a conexão MQTT não estiver ativa, tenta reconectar
  }
  client.loop(); // Mantém a conexão MQTT ativa e lida com mensagens MQTT recebidas

  unsigned long now = millis();
  
  if (now - lastMsg > 1000) { // Verifica se passou um segundo desde a última publicação
    lastMsg = now; // Atualiza o registro do tempo da última publicação
    TempAndHumidity  data = dht.getTempAndHumidity(); // Lê a temperatura e umidade do sensor DHT22

    String temp = String(data.temperature, 2); // Converte a temperatura em uma string com 2 casas decimais
    client.publish("/gubalero/temp", temp.c_str()); // Publica a temperatura em um tópico MQTT
    String hum = String(data.humidity, 1); // Converte a umidade em uma string com 1 casa decimal
    client.publish("/gubalero/hum", hum.c_str()); // Publica a umidade em um tópico MQTT

    Serial.print("Temperatura: ");
    Serial.println(temp); // Exibe a temperatura no monitor serial
    Serial.print("Umidade: ");
    Serial.println(hum); // Exibe a umidade no monitor serial
  }
}