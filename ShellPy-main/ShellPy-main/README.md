# ShellPy
Projet de fin de semestre du module "Langage Script" en L3 Informatique parcours Administration Système en Réseaux co-réalisé avec Mehnana Ilyes. L'objectif est de développer un mini interpréteur de commandes similaire à un Shell Unix basique.

# Projet Shell Python – Rendu 1 : Noyau minimal et fonctionnalités

## Groupe
- Nom du groupe : D. Xebec
- Membres :
  - Laghsal Riwan – 20233810
  - Mehnana Ilyes – 20234622

## Sujet
Développement d’un mini shell Unix en Python, capable d’être utilisé en mode interactif et en mode script, avec support progressif des fonctionnalités essentielles d’un interpréteur de commandes.

---

## 1. Fonctionnalités indispensables (noyau minimal)

| ID | Fonctionnalité | Description | Dépendances |
|----|----------------|-------------|--------------|
| #1 | Lecture d'une ligne de commane - Parsing basique | Découper la ligne en tokens (commandes + arguments) | Aucune |
| #2 | Exécution de programmes externes | Permet d'exécuter des commandes comme `ls`, `cat`, `rm`, etc. | #1 |
| #3 | Commandes internes de base | Implémentation de `cd`, `if`, `for` etc. | #1 |

✅ À la fin de #1 → #5, on a un shell fonctionnel minimal.

---

## 2. Fonctionnalités additionnelles

| ID | Fonctionnalité | Description | Dépendances |
|----|----------------|-------------|--------------|
| #4 | Variables | Affectation et expansion : `VAR=val` puis `echo $VAR` | #1 |
| #5 | Redirections | `>`, `>>`, `<` pour rediriger entrée/sortie | #2 |
| #6 | Pipes | `cmd1 \| cmd2` permettant de connecter plusieurs commandes | #2, #5 |
| #7 | Gestion du background | Exécution d’un programme avec `&` sans bloquer le shell (usage de `fork()`) | #2 |
| #8 | Mode script | Lire un fichier et exécuter les commandes ligne par ligne | #1, #2, #3 |
| #9 | Commande interne `help` | Affiche la liste des commandes internes disponibles | #1, #3 |
| #10 | Historique navigable des commandes | Mémoriser commandes précédentes et naviguer à l'aide des flèches directionelles  | #1 |
| #11 | Bannière ShellPy + clear screen | Au lancement, l’écran est nettoyé et une bannière ASCII est affichée | Aucun |
| #12 | Invite de commande personnalisée | Affichage de `ShellPy:/chemin >` en couleur au lieu de `$` | #1 |

---

## 3. Priorisation

| Priorité | Fonctionnalités |
|----------|-----------------|
| 🎯 **Haute (obligatoire – noyau minimal)** | #1, #2, #3|
| ✅ **Moyenne (fonctionnalités usuelles d’un shell)** | #4, #5, #6, #7, #8 |
| ⭐ **Faible (extras / confort utilisateur)** | #9, #10, #11, #12 |

---

## Projet ShellPy – Rendu 2 : Briques logicielles envisagées

### Noyau minimal

1. **Lecture d'une ligne de commande – Parsing basique**  
   - `builtins & shlex` – Pour utiliser les fonctions `strip` et `shlex.split` simplement.

2. **Exécution de programmes externes**  
   - `subprocess` – Permet d’exécuter des commandes systèmes externes.

3. **Commandes internes de base**  
   - `os`, `sys`, `pathlib` – Pour naviguer dans les dossiers (`cd`) et gérer l’environnement.

---

### Fonctionnalités supplémentaires

4. **Variables (`VAR=val`, `$VAR`)**  
   - `os` – Pour accéder et modifier les variables d’environnement.

5. **Redirections (`>`, `>>`, `<`)**  
   - `subprocess`, `os` – Pour rediriger les entrées/sorties de fichiers.

6. **Pipes (`|`)**  
   - `subprocess` – Pour chaîner plusieurs processus avec des flux de données.

7. **Exécution en arrière-plan (`&`)**  
   - `subprocess` – Pour lancer un processus sans attendre son achèvement.

8. **Mode script**  
   - `os`, `sys` – Pour lire et exécuter un fichier ligne par ligne.

9. **Commande `help`**  
   - `builtins` – Pour afficher un texte d’aide en interne simplement.

10. **Historique avec les flèches**  
   - `readline` *(sous Linux)* – Pour naviguer dans l’historique des commandes.

11. **Bannière ASCII + Clear screen**  
   - `os` – Pour effacer l’écran avec `clear` et afficher une bannière.

12. **Invite personnalisée (`ShellPy:/chemin >`)**  
   - `os` – Pour afficher le répertoire courant dans le prompt.

---

# Rendu final - Projet Shell - Groupe D.Xebec

---

## 📦 Bibliothèques Python utilisées

- **os** : gestion des fonctionnalités du système d'exploitation.
- **sys** : gestion des entrées/sorties et codes de retour.
- **subprocess** : execution des commandes externes.
- **shlex** : parsing (découpage lignes de commandes)
- **re** : tokenizer (remplacement de caractères, placement d'alertes)
- **readline** : historique navigable des commandes

Toutes ces bibliothèques font partie de la bibliothèque standard de Python.

---

## 🧱 Structure du projet

- ├── main.py # Point d'entrée du shell
- ├── src
- │ ├── cmd_built_in/ # Commandes internes (cd, if, for, unset, exit, help)
- │ ├── execution/ # Exécution des commandes, redirections, pipelines, background
- │ ├── parsing/ # Analyse syntaxique pour tokenizer et expansion et affectation des variables
- │ └── utils/ # Composants d'état, historique, affichage, gestion builtins

---

## 👥 Répartition du travail

- Laghsal Riwan : Parsing, expansion et affectation de variables, execution de pipelines, redirections et background, commandes builtins help et unset, gestions des utils (état du shell, affichage d'écran), vérification d'états des processus dans le main.
- Ilyes Mehnana : Commandes builtins (cd, if, for), execution de commandes simple, gestion des builtins dans utils, boucle principale du main (lire, coordonner et lancer les entrées utilisateurs).
- Travail commun : Dans les fichiers `main.py` et `executor_simple.py`.

| Laghsal Riwan | Mehnana Ilyes |
|----|----------------|
|parser.py|cd.py|
|tokenizer.py|func_for.py|
|expand_var.py|func_if.py|
|redir.py|executor_simple.py|
|exec_pipeline.py|handle_builtins.py|
|help.py|
|unset.py|
|background.py|
|print_screen.py|
|shell_state.py|
|users_var.py|


