#include <iostream>
#include <fstream>
using namespace std;
#include "Interpreteur.h"
#include "Exceptions.h"

int main(int argc, char* argv[]) {
  string nomFich="TestRepeter.txt";
  if (argc != 2) {
    cout << "Usage : " << argv[0] << " nom_fichier_source" << endl << endl;
    //cout << "Entrez le nom du fichier que voulez-vous interpréter : ";
    //getline(cin, nomFich);
  } else
    nomFich = argv[1];
  ifstream fichier(nomFich.c_str());
  try {
    Interpreteur interpreteur(fichier);
    
    interpreteur.analyse();
    
    if (interpreteur.sansErreur()) {
       // Si pas d'exception levée, l'analyse syntaxique a réussi
        cout << endl << "================ Syntaxe Correcte" << endl;
        // On affiche le contenu de la table des symboles avant d'exécuter le programme
        cout << endl << "================ Table des symboles avant exécution : " << interpreteur.getTable();
        cout << endl << "================ Execution de l'arbre" << endl;
        // On exécute le programme si l'arbre n'est pas vide
        if (interpreteur.getArbre()!=nullptr) interpreteur.getArbre()->executer();
        // Et on vérifie qu'il a fonctionné en regardant comment il a modifié la table des symboles
        cout << endl << "================ Table des symboles apres exécution : " << interpreteur.getTable(); 
        
        //Traduction
        ofstream fichier("traduction.txt", ios::out | ios::trunc);
        if(fichier) {
            interpreteur.traduitEnCPP(fichier,1);
            fichier.close();  // on ferme le fichier
        } else  // sinon
            cout << "Impossible d'ouvrir le fichier !" << endl;
        
    } else {
        interpreteur.afficherErreur();
    }
    
  } catch (InterpreteurException & e) {
    cout << e.what() << endl;
  }
  
  return 0;
}
