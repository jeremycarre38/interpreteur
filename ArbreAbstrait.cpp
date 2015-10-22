#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"
#include "TableSymboles.h"
#include <typeinfo>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}

int NoeudSeqInst::executer() {
  for (unsigned int i = 0; i < m_instructions.size(); i++)
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
  if (instruction!=nullptr) m_instructions.push_back(instruction);
}

void NoeudSeqInst::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    for (Noeud* instruction : m_instructions) {
        instruction->traduitEnCPP(cout,indentation);//indente toutes les autres instructions !
        cout << endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
  int valeur = m_expression->executer(); // On exécute (évalue) l'expression
  ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudAffectation::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4*indentation) << "";
    m_variable->traduitEnCPP(cout, 0);
    cout << " = ";
    m_expression->traduitEnCPP(cout, 0);
    cout << ";";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
  int og, od, valeur;
  if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
  if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
  // Et on combine les deux opérandes en fonctions de l'opérateur
  if (this->m_operateur == "+") valeur = (og + od);
  else if (this->m_operateur == "-") valeur = (og - od);
  else if (this->m_operateur == "*") valeur = (og * od);
  else if (this->m_operateur == "==") valeur = (og == od);
  else if (this->m_operateur == "!=") valeur = (og != od);
  else if (this->m_operateur == "<") valeur = (og < od);
  else if (this->m_operateur == ">") valeur = (og > od);
  else if (this->m_operateur == "<=") valeur = (og <= od);
  else if (this->m_operateur == ">=") valeur = (og >= od);
  else if (this->m_operateur == "et") valeur = (og && od);
  else if (this->m_operateur == "ou") valeur = (og || od);
  else if (this->m_operateur == "non") valeur = (!og);
  else if (this->m_operateur == "/") {
    if (od == 0) throw DivParZeroException();
    valeur = og / od;
  }
  return valeur; // On retourne la valeur calculée
}

void NoeudOperateurBinaire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << "";
    m_operandeGauche->traduitEnCPP(cout,indentation);
    cout << " " << m_operateur.getChaine() << " ";
    m_operandeDroit->traduitEnCPP(cout,indentation);
    //cout << ";";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstSi::executer() {
  if (m_condition->executer()) m_sequence->executer();
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstSi::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4*indentation)<<""<< "if ("; //Ecrit "if (" avec un décalage de 4 indentations
    m_condition->traduitEnCPP(cout,0);
    cout << ") {" << endl;
    m_sequence->traduitEnCPP(cout, indentation+1);
    cout << setw(4*indentation) << "" << "}" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTantQue::executer() {
    while(m_condition->executer()) m_sequence->executer();
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstTantQue::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4*indentation) <<"" << "while (";
    m_condition->traduitEnCPP(cout,0);
    cout << ") {" << endl;
    m_sequence->traduitEnCPP(cout, indentation+1);
    cout << setw(4*indentation) << "" << "}" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudInstRepeter::NoeudInstRepeter(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstRepeter::executer() {
    do m_sequence->executer(); 
    while (m_condition->executer());
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstRepeter::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4*indentation) <<"" << "do {" << endl;
    m_sequence->traduitEnCPP(cout, indentation+1);
    cout << setw(4*indentation) <<"" << "} while (";
    m_condition->traduitEnCPP(cout, 0);
    cout << ");" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(Noeud* variable, Noeud* condition, Noeud* increment, Noeud* sequence)
: m_variable(variable),m_condition(condition),m_incrementeur(increment), m_sequence(sequence) {
}

int NoeudInstPour::executer() {
    
    for (m_variable->executer();m_condition->executer();m_incrementeur->executer()) {
        m_sequence->executer();
    }
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstPour::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4*indentation) << "" << "for (";
    m_variable->traduitEnCPP(cout,0);
    //cout << ";";
    m_condition->traduitEnCPP(cout,0);
    //cout << ";";
    m_incrementeur->traduitEnCPP(cout, 0);
    cout << ") {" << endl;
    m_sequence->traduitEnCPP(cout,indentation+1);
    cout << setw(4*indentation) << "" << "}" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstEcrire
////////////////////////////////////////////////////////////////////////////////

NoeudInstEcrire::NoeudInstEcrire(vector<Noeud*> expression)
:m_expression(expression) {
}

int NoeudInstEcrire::executer() {
    for (Noeud* ptr : m_expression) {
        if ( (typeid(*ptr)==typeid(SymboleValue) &&  *((SymboleValue*)ptr)== "<CHAINE>" )) {
            cout << ((SymboleValue*) ptr)->getChaine();
        } else {
            cout << ptr->executer();
        }
    }
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstEcrire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4*indentation) << "" << "cout";
    for (Noeud* monExp : m_expression) {
        cout << " << ";
        monExp->traduitEnCPP(cout,0);
    }
    cout << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstLire
////////////////////////////////////////////////////////////////////////////////

NoeudInstLire::NoeudInstLire(vector<Noeud*> expression)
:m_expression(expression) {
}

int NoeudInstLire::executer() {
    for (Noeud* ptr : m_expression) {
        int temp=0;
        cin >> temp;
        ((SymboleValue*)ptr)->setValeur(temp);
    }
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstLire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4*indentation) << ""<< "cin";
    for (Noeud* monExp : m_expression) {
        cout << " >> ";
        monExp->traduitEnCPP(cout,0);
    }
    cout << endl;
}