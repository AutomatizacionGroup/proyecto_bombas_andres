#include "Internet.h"

const char* paginaHTML = "<!DOCTYPE html>"
                            "<html>"
                            "<head>"
                        "<title>Sistema de Bombeo</title>"
                            "</head>"
                            "<body>"

                        "<h1>Esto es una pagina ejemplo</h1>"
                        "<p>Hecha con el proposito de probar el sistema</p>"

                            "</body>"
                            "</html>"
;


//CONSTRUCTOR PARA OPERACION COMO ACCESS POINT  
Internet::Internet(String ssid, String pass){
    WiFiServer server(80); //crea servidor en puerto 80
    ssid_ap = ssid;
    pass_ap = pass;
}

//CONSTRUCTOR PARA OPERACION DE ESTADO ESTACIONARIO
Internet::Internet(String ssid, String pass, String mqtt_us, String mqtt_p, String mqtt_add){
    WiFiClient c;
    PubSubClient cliente(c);
    ssid_hn = ssid;
    pass_hn = pass;
    mqtt_user = mqtt_us;
    mqtt_pass = mqtt_p;
    mqtt_ip = mqtt_add;
}

//CONFIGURA Y CREA EL ACCESS POINT
void Internet::crear_accessp(String ssid, String pass){
    Serial.print("creando punto de acceso …");
    while(!WiFi.softAP(ssid.c_str(), pass.c_str())){
        Serial.print(".");
        delay(500);
    }

    while(!WiFi.softAPConfig(IPAddress(192,168,0,1),IPAddress(192,168,0,10), IPAddress(255,255,255,0))){
        Serial.println("configuracion de access point fallida");
    }

}

//PAGINA DE SERVIDOR WEB
void Internet::pagina(){
    WiFiClient client = server.available();
    while(!client){
        Serial.println("sin conexion de un cliente");
        Serial.println(WiFi.softAPIP());
        delay(1000);
    }
    if (client){
        Serial.println("cliente ha conectado");
        while(client.connected()){
            if (client.available()){
                client.println(paginaHTML);
                //manejar trafico, etc.
            }
        }
    }
}

//TUMBAR ACCESS POINT
void Internet::kill_accessp(){
    WiFi.softAPdisconnect(true);
    server.~WiFiServer();
}

//CONECTAR A RED LOCAL DE WIFI  
void Internet::conectar_WIFI(){
    delay(10);
    Serial.print("conectandose a: ");
    Serial.println(ssid_hn);

    //inicia proceso de conexion
    WiFi.begin(ssid_hn.c_str(), pass_hn.c_str());

    //si no conecta de una vez reintenta conexion cada medio segundo
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi conectado exitosamente");
    Serial.println("Direccion ip: ");
    Serial.println(WiFi.localIP());
}

void Internet::subscribir_MQTT(){
    cliente.subscribe("sistema-bombas/boost");
    cliente.subscribe("sistema-bombas/nivel-tanque");
}

//CONECTAR A SERVIDOR MQTT
void Internet::conectar_MQTT(){
    while(!cliente.connected()){
        //id de cliente
        const char* clientId = "esp32_client";
        //intento de conectar
        if(cliente.connect(clientId,mqtt_user.c_str(),mqtt_pass.c_str())){
            //Subscripciones relevantes
            subscribir_MQTT();
        }
        else{
            //esperar 5 segundos antes de reintentar
            delay(5000);
        }
    }
}

//PUBLICAR MQTT 
void Internet::publicar(char topico, String mensaje){
    switch (topico){
        case 'p': //p es presion de salida
            cliente.publish("sistema-bombas/presion-salida", mensaje.c_str());
            break;
        case 'a': //a es entrada de agua
            cliente.publish("sistema-bombas/entrada-agua", mensaje.c_str());
            break;
        case '1':
            cliente.publish("sistema-bombas/falla-1", mensaje.c_str());
            break;
        case '2':
            cliente.publish("sistema-bombas/falla-2", mensaje.c_str());
        default:
            break;
    }
}

Internet::~Internet(){}