#include "network.hpp"
#include <iostream>

int main() {
    NetworkManager manager(12345);  // Port 12345
    manager.Start();               // Démarrer
    
    std::cout << "Serveur en attente de clients..." << std::endl;
    
    // Boucle principale
   while (true) {
        manager.Update();  // Traiter les événements
        // Ajouter d'autres trucs du serveur ici
    }
    
    manager.Stop();
    return 0;
}