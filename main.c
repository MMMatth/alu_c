/*
 * -------------------------- TP du module Archi -------------------------
 *
 * ATTENTION : un outil de détection de plagiat logiciel sera utilisé lors de la
 * correction, vous avez donc tout intérêt à effectuer un travail PERSONNEL
 *
 * Un mot/registre de NBITS bits (par défaut NBITS=16) est représenté par un
 * tableau d'entiers égaux à 0 ou 1. Une ALU est représentée par une structure
 * ALU, avec registre accumulateur et registre d'état. Un CPU (très très
 * simplifié) est représenté par une ALU et quelques registres nécessaires pour
 * stocker les résultats intermédiaires.
 *
 * Certaines fonctions vous sont fournies, d'autres sont à implanter ou à
 * compléter, de préférence dans l'ordre où eles sont indiquées. Il vous est
 * fortement conseillé de lire attentivement l'ensemble des commentaires.
 *
 * Parmi les opérations arithmétiques et logiques, seules 4 opérations de base
 * sont directement fournies par l'ALU, les autres doivent être décrites comme
 * des algorithmes travaillant à l'aide des opérateurs de base de l'ALU
 * simplifiée et pouvant utiliser les registres du CPU.
 *
 * La fonction main() vous permet de tester au fur et à mesure les fonctions que
 * vous implantez.
 *
 * ----------------------------------------------------------------------------------------------
 *
 * author: B. Girau
 * version: 2021-22
 */
#include <stdio.h>
#include <stdlib.h>

#define NBITS                                                                  \
  16 // attention, votre programme doit pouvoir être adapté à d'autres tailles
     // juste en modifiant la valeur de cette constante
// en ayant toujours NBITS < 32

/////////////////////////////////////////////////////////
// définition de types
/////////////////////////////////////////////////////////

typedef struct {
  int *accu;  // stocke le résultat des opérations (NBITS bits)
  int *flags; // indicateurs ZF CF OF NF
} ALU;

typedef struct {
  ALU alu;
  int *R0; // registres de travail (NBITS bits)
  int *R1;
  int *R2;
} CPU;

/////////////////////////////////////////////////////////
// fonctions d'initialisation
/////////////////////////////////////////////////////////

/*
 * allocation d'un mot entier de NBITS bits initialisé à 0
 */
int *word() {
  int *tab;
  int i;
  tab = (int *)malloc(NBITS * sizeof(int));
  for (i = 0; i < NBITS; i++)
    tab[i] = 0;
  // poids faible : tab[0]
  // poids fort : tab[NBITS-1]
  return tab;
}

/*
 * Initialisation du mot (mot de NBITS bits, codant un entier en Cà2) avec une
 * valeur entière.
 */
void setValue(int *word, int n) {
  // word[0] = poids faible
  // word[NBITS-1] = poids fort
  // il faut mettre l'entier n dans le mot word en complement a 2
  int temp = n; // pour gérer le cas n < 0
  if (n < 0)
    temp = -n; // on travaille sur la valeur absolue de n
  for (int i = 0; i < NBITS; i++) {
    word[i] = temp % 2;
    temp = temp / 2;
  }
  if (n < 0) {
    int i = 0;
    while (word[i] != 1)
      i++;
    i++;
    for (int j = i; j < NBITS; j++) {
      if (word[j] == 1)
        word[j] = 0;
      else
        word[j] = 1;
    }
  }
}

/*
 * instanciation d'un mot de NBITS bits initialisé avec la valeur n
 */
int *initWord(int n) {
  int *tab = word();
  setValue(tab, n);
  return tab;
}

/*
 * Initialisation du mot (mot de NBITS bits) par recopie des bits du mot en
 * paramètre.
 */
void copyValue(int *word, int *src) {
  for (int i = 0; i < NBITS; i++) {
    word[i] = src[i];
  }
}

/*
 * instanciation d'un mot de NBITS bits initialisé par recopie d'un mot
 */
int *copyWord(int *src) {
  int *tab = word();
  copyValue(tab, src);
  return tab;
}

/*
 * initialise l'ALU
 */
ALU initALU() {
  ALU res;
  res.accu = word();
  res.flags = (int *)malloc(4 * sizeof(int));
  return res;
}

/*
 * initialise le CPU
 */
CPU initCPU() {
  CPU res;
  res.alu = initALU();
  res.R0 = (int *)malloc(NBITS * sizeof(int));
  res.R1 = (int *)malloc(NBITS * sizeof(int));
  res.R2 = (int *)malloc(NBITS * sizeof(int));
  return res;
}

/////////////////////////////////////////////////////////
// fonctions de lecture
/////////////////////////////////////////////////////////

/*
 * Retourne la valeur entière signée représentée par le mot (complément à 2).
 */
int intValue(int *word) {
  int res = 0;
  int *temp = copyWord(word);
  if (temp[NBITS - 1] == 1) {
    int i = 0;
    while (temp[i] != 1)
      i++;
    i++;
    for (int j = i; j < NBITS; j++) {
      if (temp[j] == 1)
        temp[j] = 0;
      else
        temp[j] = 1;
    }
  }
  for (int i = 0; i < NBITS; i++) {
    res += temp[i] * (1 << i);
  }
  if (word[NBITS - 1] == 1)
    res = -res;

  return res;
}

/*
 * Retourne une chaîne de caractères décrivant les NBITS bits
 */
char *toString(int *word) {
  int i, j = 0;
  char *s = (char *)malloc((2 + NBITS) * sizeof(char));
  for (i = NBITS - 1; i >= 0; i--) {
    if (word[i] == 1)
      s[j] = '1';
    else
      s[j] = '0';
    j++;
  }
  s[j] = 0;
  return s;
}

/*
 * Retourne l'écriture des indicateurs associés à l'ALU.
 */
char *flagsToString(ALU alu) {
  char *string = (char *)malloc(10 * sizeof(char));
  sprintf(string, "z%dc%do%dn%d", alu.flags[0], alu.flags[1], alu.flags[2],
          alu.flags[3]);
  return string;
}

/*
 * affiche à l'écran le contenu d'une ALU
 */
void printing(ALU alu) {
  printf("accu = %s\n", toString(alu.accu));
  printf("flags = %s\n", flagsToString(alu));
}

/////////////////////////////////////////////////////////
// fonctions de manipulations élémentaires
/////////////////////////////////////////////////////////

/*
 * Mise à la valeur b du bit spécifié dans le mot
 */
void set(int *word, int bitIndex, int b) {
  if ((bitIndex > NBITS - 1) || (bitIndex < 0))
    printf("valeur d'index incorrecte\n");
  word[bitIndex] = b;
}

/*
 * Retourne la valeur du bit spécifié dans le mot
 */
int get(int *word, int bitIndex) {
  if ((bitIndex > NBITS - 1) || (bitIndex < 0))
    printf("valeur d'index incorrecte\n");
  return word[bitIndex];
}

/*
 * Positionne l'indicateur ZF en fonction de l'état de l'accumulateur
 */
void setZ(ALU alu) {
  int i = 0;
  while ((i < NBITS) && (alu.accu[i] == 0))
    i++;
  if (i == NBITS) // si tous les bits sont à 0
    alu.flags[0] = 1;
  else // si au moins un bit est à 1
    alu.flags[0] = 0;
}

/////////////////////////////////////////////////////////
// opérateurs de base de l'ALU
// IMPORTANT : les indicateurs doivent être mis à jour
/////////////////////////////////////////////////////////

/*
 * Stocke le paramètre dans le registre accumulateur
 */
void pass(ALU alu, int *B) {
  // à compléter
  copyValue(alu.accu, B);
  setZ(alu);
}

/*
 * Effectue un NAND (NON-ET) entre le contenu de l'accumulateur et le paramètre.
 */
void nand(ALU alu, int *B) {
  for (int i = 0; i < NBITS; i++) {
    if ((alu.accu[i] == 1) && (B[i] == 1))
      alu.accu[i] = 0;
    else
      alu.accu[i] = 1;
  }
}

/*
 * Décale le contenu de l'accumulateur de 1 bit vers la droite
 */
void shift(ALU alu) {
  /* first we store the output byte */
  alu.flags[1] = alu.accu[0];
  for (int i = 0; i < NBITS - 1; i++) {
    // we shift the bits
    alu.accu[i] = alu.accu[i + 1];
  }
  // we add the last bit
  alu.accu[NBITS - 1] = 0;
}

/*
 * module Full Adder : a+b+c_in = s + 2 c_out
 * retourne un tableau contenant s et c_out
 */
int *fullAdder(int a, int b, int c_in) {
  int *res = (int *)malloc(2 * sizeof(int));
  res[0] = a ^ b ^ c_in;               // S
  res[1] = (a & b) | (c_in & (a ^ b)); // Cout
  return res;
}

/*
 * Additionne le paramètre au contenu de l'accumulateur (addition entière Cà2).
 * Les indicateurs sont positionnés conformément au résultat de l'opération.
 */
void add(ALU alu, int *B) {
  for (int i = 0; i < NBITS; i++) {
    int *temp = fullAdder(alu.accu[i], B[i], alu.flags[1]);
    alu.accu[i] = temp[0];
    alu.flags[1] = temp[1];
  }
}

////////////////////////////////////////////////////////////////////
// Opérations logiques :
////////////////////////////////////////////////////////////////////

/*
 * Négation.
 */
void not(CPU cpu) { nand(cpu.alu, cpu.alu.accu); }

/*
 * Et.
 */
void and (CPU cpu, int *B) {
  nand(cpu.alu, B);            // nand(A,B)
  nand(cpu.alu, cpu.alu.accu); // nand(nand(A,B),nand(A,B))
}

/*
 * Ou.
 */
void or (CPU cpu, int *B) {
  cpu.R1 = copyWord(cpu.alu.accu); // R1 = A
  cpu.R2 = copyWord(B);            // R2 = B

  nand(cpu.alu, cpu.R1); // nand(A,A)
  cpu.R1 = cpu.alu.accu; // R1 = nand(A,A)

  cpu.alu.accu = cpu.R2; // i place B in the acc
  nand(cpu.alu, cpu.R2); // nand(B,B)
  cpu.R2 = cpu.alu.accu; // R2 = nand(B,B)

  cpu.alu.accu = cpu.R1; // i recover nand(A,A)
  nand(cpu.alu, cpu.R2); // nand(nand(A,A),nand(B,B))
}

/*
 * Xor.
 */
void xor
    (CPU cpu, int *B) {
      // A xor B = (A ou B) et non (A et B)
      /* first we save A B*/
      cpu.R1 = copyWord(cpu.alu.accu); // we save the accu in R1
      cpu.R2 = copyWord(B);            // we save B in R2

      /* we compute A or B */
      or (cpu, cpu.R2);      // or(A,B)
      cpu.R0 = cpu.alu.accu; // R0 = or(A,B)
      printf("or(A,B) = %s\n", toString(cpu.alu.accu));

      /* we compute A and B */
      cpu.alu.accu = cpu.R1; // i recover A
      nand(cpu.alu, cpu.R2); // nand(A,B)
      cpu.R1 = cpu.alu.accu; // R1 = nand(A,B)
      printf("nand(A,B) = %s\n", toString(cpu.alu.accu));

      /* we compute (a or b) and not (a et b)*/
      cpu.alu.accu = cpu.R0; // i recover or(A,B)
      and(cpu, cpu.R1);      // and(or(A,B),not(and(A,B)))
    }

    /*
     * Décale le receveur de |n| positions.
     * Le décalage s'effectue vers la gauche si n>0 vers la droite dans le cas
     * contraire. C'est un décalage logique (pas de report du bit de signe dans
     * les positions libérées en cas de décalage à droite). L'indicateur CF est
     * positionné avec le dernier bit "perdu".
     */
    void logicalShift(CPU cpu, int n) {
  if (n > 0) {
    shift(cpu.alu);
    logicalShift(cpu, n - 1);
  }
}

/////////////////////////////////////////////////////////
// Opérations arithmétiques entières
/////////////////////////////////////////////////////////

/*
 * Opposé.
 */
void opp(CPU cpu) {
  not(cpu);                  // not(A)
  add(cpu.alu, initWord(1)); // not(A) + 1
}

/*
 * Soustraction.
 */
void sub(CPU cpu, int *B) {
  // à compléter
}

/*
 * Multiplication.
 */
void mul(CPU cpu, int *B) {
  // à compléter
}

/////////////////////////////////////////////////////////
// Programme de test
/////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  /*
  Ce programme est fourni à titre d'exemple pour permettre de tester simplement
  vos fonctions. Il vous est bien entendu possible de le modifier/compléter, ou
  encore d'écrire vos propres fonctions de test.
  */

  int *operand;
  ALU alu;
  CPU cpu;

  int chosenInt, integer;
  int go_on = 1;

  char *menu = "              Programme de test\n\n0  Quitter\n1  "
               "setValue(operande,int)\n2  pass(alu,operande)\n3  "
               "printing(alu)\n4  afficher toString(operande)\n5  afficher "
               "intValue(operande)\n6  afficher intValue(accu)\n7  "
               "accu=nand(accu,operande)\n8  accu=add(accu,operande)\n9  "
               "accu=sub(accu,operande)\n10  accu=and(accu,operande)\n11 "
               "accu=or(accu,operande)\n12 accu=xor(accu,operande)\n13 "
               "accu=not(accu)\n14 accu=opp(accu)\n15 accu=shift(accu)\n16 "
               "accu=logicalShift(accu,int)\n17 accu=mul(accu,operande)\n\n";

  char *invite = "--> Quel est votre choix  ? ";

  printf("%s", menu);

  operand = word();
  cpu = initCPU();
  alu = cpu.alu;

  while (go_on == 1) {
    printf("%s", invite);
    scanf("%d", &chosenInt);
    switch (chosenInt) {
    case 0:
      go_on = 0;
      break;
    case 1:
      printf("Entrez un nombre :");
      scanf("%d", &integer);
      setValue(operand, integer);
      break;
    case 2:
      pass(alu, operand);
      break;
    case 3:
      printing(alu);
      break;
    case 4:
      printf("%s\n", toString(operand));
      break;
    case 5:
      printf("intValue(operand)=%d\n", intValue(operand));
      break;
    case 6:
      printf("intValue(accu)=%d\n", intValue(alu.accu));
      break;
    case 7:
      nand(alu, operand);
      printf("apres nand(), accu = ");
      printing(alu);
      break;
    case 8:
      add(alu, operand);
      printf("apres add(), accu = ");
      printing(alu);
      break;
    case 9:
      sub(cpu, operand);
      printf("apres sub(), A = ");
      printing(alu);
      break;
    case 10:
      and(cpu, operand);
      printf("apres and(), A = ");
      printing(alu);
      break;
    case 11:
      or (cpu, operand);
      printf("apres or(), A = ");
      printing(alu);
      break;
    case 12:
      xor(cpu, operand);
      printf("apres xor(), A = ");
      printing(alu);
      break;
    case 13:
      not(cpu);
      printf("apres not(), A = ");
      printing(alu);
      break;
    case 14:
      opp(cpu);
      printf("apres opp(), A = ");
      printing(alu);
      break;
    case 15:
      shift(alu);
      printf("apres shift(), A = ");
      printing(alu);
      break;
    case 16:
      printf("Entrez un entier :");
      scanf("%d", &integer);
      logicalShift(cpu, integer);
      printf("apres logicalshift(%d), A = ", integer);
      printing(alu);
      break;
    case 17:
      mul(cpu, operand);
      printf("apres mul(), A = ");
      printing(alu);
      break;
    default:
      printf("Choix inexistant !!!!\n");
      printf("%s", menu);
    }
  }
  printf("Au revoir et a bientot\n");
  return 0;
}
