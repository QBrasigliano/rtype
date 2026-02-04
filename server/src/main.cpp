#include "network.hpp"
#include <iostream>

int main() {
    NetworkManager manager(4242);  // Port 4242
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