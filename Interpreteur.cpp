#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <sstream> 
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr) {
}

void Interpreteur::analyse() {
    m_arbre = programme(); // on lance l'analyse de la première règle
}

void Interpreteur::tester(const string & symboleAttendu) const throw (SyntaxeException) {
    // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
    stringstream messageWhat;
    if (m_lecteur.getSymbole() != symboleAttendu) {
        messageWhat << "Ligne" << m_lecteur.getLigne() << ", Colonne" << m_lecteur.getColonne() << " - Erreur de syntaxe - Symbole attendu : "
                << symboleAttendu.c_str() << " - Symbole trouvé : " << m_lecteur.getSymbole().getChaine().c_str();

        char* msgC;
        string msg = messageWhat.str();
        msgC = (char*) msg.c_str();

        throw SyntaxeException(msgC);
    }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException) {
    // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
    //try {
        tester(symboleAttendu);
    //} catch (SyntaxeException e) {
    //    m_exception.push_back(e);
    //}


    m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const throw (SyntaxeException) {
    // Lève une exception contenant le message et le symbole courant trouvé
    // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
    static char messageWhat[256];
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() {
    // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    testerEtAvancer("procedure");
    testerEtAvancer("principale");
    testerEtAvancer("(");
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    testerEtAvancer("finproc");
    tester("<FINDEFICHIER>");
    return sequence;
}

Noeud* Interpreteur::seqInst() {
    // <seqInst> ::= <inst> { <inst> }
    NoeudSeqInst* sequence = new NoeudSeqInst();
    do {
        sequence->ajoute(inst());
    } while (m_lecteur.getSymbole() == "<VARIABLE>" ||
            m_lecteur.getSymbole() == "si" ||
            m_lecteur.getSymbole() == "tantque" ||
            m_lecteur.getSymbole() == "repeter" ||
            m_lecteur.getSymbole() == "pour" ||
            m_lecteur.getSymbole() == "ecrire" ||
            m_lecteur.getSymbole() == "lire");
    // Tant que le symbole courant est un début possible d'instruction...
    // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
    return sequence;
}

Noeud* Interpreteur::inst() {
    // <inst> ::= <affectation>  ; | <instSi> | <instTantQue> | <instRepeter> | <instPour> | <instEcrire>
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        Noeud *affect = affectation();
        testerEtAvancer(";");
        return affect;
    } else if (m_lecteur.getSymbole() == "si")
        return instSi();
        // Compléter les alternatives chaque fois qu'on rajoute une nouvelle instruction
    else if (m_lecteur.getSymbole() == "tantque")
        return instTantQue();
    else if (m_lecteur.getSymbole() == "repeter")
        return instRepeter();
    else if (m_lecteur.getSymbole() == "pour")
        return instPour();
    else if (m_lecteur.getSymbole() == "ecrire")
        return instEcrire();
    else if (m_lecteur.getSymbole() == "lire")
        return instLire();
    else cout << "instruction non trouvé";
        //erreur("Instruction incorrecte");
}

Noeud* Interpreteur::affectation() {
    // <affectation> ::= <variable> = <expression> 
    tester("<VARIABLE>");
    Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table et on la mémorise
    m_lecteur.avancer();
    testerEtAvancer("=");
    Noeud* exp = expression(); // On mémorise l'expression trouvée
    return new NoeudAffectation(var, exp); // On renvoie un noeud affectation
}

/*Noeud* Interpreteur::expression() {
    // <expression> ::= <facteur> { <opBinaire> <facteur> }
    //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
    Noeud* fact = facteur();
    while (m_lecteur.getSymbole() == "+" || m_lecteur.getSymbole() == "-" ||
            m_lecteur.getSymbole() == "*" || m_lecteur.getSymbole() == "/" ||
            m_lecteur.getSymbole() == "<" || m_lecteur.getSymbole() == "<=" ||
            m_lecteur.getSymbole() == ">" || m_lecteur.getSymbole() == ">=" ||
            m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
            m_lecteur.getSymbole() == "et" || m_lecteur.getSymbole() == "ou") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* factDroit = facteur(); // On mémorise l'opérande droit
        fact = new NoeudOperateurBinaire(operateur, fact, factDroit); // Et on construuit un noeud opérateur binaire
    }
    return fact; // On renvoie fact qui pointe sur la racine de l'expression
}*/

Noeud* Interpreteur::expression() {
    // <expression> ::= <terme> { + <terme> | - <terme>}
    Noeud* term = terme();
    while (m_lecteur.getSymbole() == "+" || m_lecteur.getSymbole() == "-") {
        Symbole operateur = m_lecteur.getSymbole();
        m_lecteur.avancer();
        Noeud* termDroit = terme();
        term = new NoeudOperateurBinaire(operateur, term, termDroit);
    }
    cout << m_lecteur.getSymbole().getChaine();
    return term;
}

Noeud* Interpreteur::terme() {
    // <terme> ::= <facteur> { * <facteur | / <facteur>}  
    Noeud* fact = facteur();
    while (m_lecteur.getSymbole() == "*" || m_lecteur.getSymbole() == "/") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* factDroit = facteur();
        fact = new NoeudOperateurBinaire(operateur, fact, factDroit);
    }
    return fact;
}

Noeud* Interpreteur::facteur() {
    // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
    Noeud* fact = nullptr;
    if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
        fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
        m_lecteur.avancer();
    } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), facteur());
    } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("non"), facteur(), nullptr);
    } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
        m_lecteur.avancer();
        fact = expression();
        testerEtAvancer(")");
    } else
        erreur("Facteur incorrect");
    return fact;
}

Noeud * Interpreteur::expBool() {
    //  <expBool> ::= <relationEt> { ou <relationEt> }
    Noeud* relatEt = relationEt();
    while (m_lecteur.getSymbole() == "ou") {
        m_lecteur.avancer();
        relatEt = new NoeudOperateurBinaire(Symbole("ou"), relationEt(), nullptr);
    }
    return relatEt;
}

Noeud * Interpreteur::relationEt() {
    //  <relationEt> ::= <relation> { et <relation> }
    Noeud* relat = relation();
    while (m_lecteur.getSymbole() == "et") {
        m_lecteur.avancer();
        relat = new NoeudOperateurBinaire(Symbole("et"), relation(), nullptr);
    }
    return relat;
}

Noeud * Interpreteur::relation() {
    //  <expression> { <opRel> <expression> }
    Noeud* expr = expression();
    while (m_lecteur.getSymbole() == "<" || m_lecteur.getSymbole() == "<=" ||
            m_lecteur.getSymbole() == ">" || m_lecteur.getSymbole() == ">=" ||
            m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=") {

        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* exprDroit = expression(); // On mémorise l'opérande droit
        expr = new NoeudOperateurBinaire(operateur, expr, exprDroit); // Et on construuit un noeud opérateur binaire
    }
    return expr; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud * Interpreteur::opRel() {
    //  <opBinaire> ::= < | > | <= | >= | == | !=
    Noeud* symb = nullptr;
    if (m_lecteur.getSymbole() == "<" || m_lecteur.getSymbole() == "<=" ||
            m_lecteur.getSymbole() == ">" || m_lecteur.getSymbole() == ">=" ||
            m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=") {
        Symbole operateur = m_lecteur.getSymbole();
        symb = new SymboleValue(operateur); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
    }
    return symb;
}

Noeud* Interpreteur::instSi() {
    // <instSi> ::= si ( <expression> ) <seqInst> finsi
    Noeud* condition=nullptr;
    Noeud* sequence=nullptr;
    try {
        testerEtAvancer("si");
    } catch (SyntaxeException se ) {
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
    
    try {
        testerEtAvancer("(");
    } catch (SyntaxeException se) {
        cout << se.what() << endl;
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
    
    try {
        condition = relation(); // On mémorise la condition
    } catch (SyntaxeException se) {
        cout << se.what() << endl;
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
    try {
        testerEtAvancer(")");
    } catch (SyntaxeException se) {
        cout << se.what() << endl;
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
    
    try {
        sequence = seqInst(); // On mémorise la séquence d'instruction
    } catch (SyntaxeException se) {
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
        
    try {
        testerEtAvancer("finsi");
    } catch (SyntaxeException se) {
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
    
    if (sansErreur()) {
        return new NoeudInstSi(condition, sequence); // Et on renvoie un noeud Instruction Si
    } else {
        m_arbre=nullptr;
    }
}

Noeud* Interpreteur::instTantQue() {
    // <instTantQue> ::= tantque ( <expression> ) <seqInst> fintantque 
    Noeud* condition=nullptr;
    Noeud* sequence=nullptr;
    try {
        testerEtAvancer("tantque");
    } catch (SyntaxeException se) {
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
    
    try {
        testerEtAvancer("(");
    } catch (SyntaxeException se) {
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
    
    try {
        condition = relation();
    } catch (SyntaxeException se) {
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
    
    try {
        testerEtAvancer(")");
    } catch (SyntaxeException se) {
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
    
    try {
        sequence = seqInst();
    } catch (SyntaxeException se) {
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
    
    try {
        testerEtAvancer("fintantque");
    } catch (SyntaxeException se) {
        m_exception.push_back(se);
        m_lecteur.avancer();
    }
    if (sansErreur()) {
        return new NoeudInstTantQue(condition, sequence);
    }
    
    //return nullptr;
}

Noeud* Interpreteur::instRepeter() {
    // <instRepeter> ::= repeter <seqInst> jusqua ( <expression> )
    testerEtAvancer("repeter");
    Noeud* sequence = seqInst();
    testerEtAvancer("jusqua");
    testerEtAvancer("(");
    Noeud* condition = relation();
    testerEtAvancer(")");
    return new NoeudInstRepeter(condition, sequence);
    //return nullptr;
}

Noeud* Interpreteur::instPour() {
    // <instPour> ::= pour ( [ <affectation> ] ; <expression> ; [ <affectation> ] ) <seqInst> finpour
    testerEtAvancer("pour");
    testerEtAvancer("(");
    Noeud* var = affectation();
    testerEtAvancer(";");
    Noeud* condition = relation();
    testerEtAvancer(";");
    Noeud* compt = affectation();
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    testerEtAvancer("finpour");
    return new NoeudInstPour(var, condition, compt, sequence);
    //return nullptr;
}

Noeud* Interpreteur::instEcrire() {
    // <instEcrire> ::= ecrire ( <expression> | <chaine> { , <expression> | <chaine> } )
    vector<Noeud*> contenu;

    testerEtAvancer("ecrire");
    testerEtAvancer("(");
    if (m_lecteur.getSymbole() == "<CHAINE>") {
        contenu.push_back(m_table.chercheAjoute(m_lecteur.getSymbole()));
        m_lecteur.avancer();
    } else {
        contenu.push_back(expression());
    }

    while (m_lecteur.getSymbole() == ",") {
        m_lecteur.avancer();
        if (m_lecteur.getSymbole() == "<CHAINE>") {
            contenu.push_back(m_table.chercheAjoute(m_lecteur.getSymbole()));
            m_lecteur.avancer();
        } else {
            contenu.push_back(expression());
        }

    }

    testerEtAvancer(")");

    return new NoeudInstEcrire(contenu);
    //return nullptr;
}

Noeud* Interpreteur::instLire() {
    // <instLire> ::= lire ( <variable> {, <variable> })
    vector<Noeud*> contenu;

    testerEtAvancer("lire");
    testerEtAvancer("(");

    contenu.push_back(m_table.chercheAjoute(m_lecteur.getSymbole()));

    m_lecteur.avancer();

    while (m_lecteur.getSymbole() == ",") {
        m_lecteur.avancer();
        contenu.push_back(m_table.chercheAjoute(m_lecteur.getSymbole()));
        m_lecteur.avancer();
    }

    testerEtAvancer(")");

    return new NoeudInstLire(contenu);
    //return nullptr;
}

bool Interpreteur::sansErreur() {
    return m_exception.empty();
}

void Interpreteur::afficherErreur() {
    for (auto e : m_exception) {
        cout << e.what() << endl;
    }
}

void Interpreteur::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4 * indentation) << "" << "int main() {" << endl;
    getArbre()->traduitEnCPP(cout, indentation+1);
    cout << setw(4 * indentation) << "" << "return 0;" << endl;
    cout << setw(4 * indentation) << "" << "}" << endl;
}