from src.utils.shell_state import shell_state


def help():
    print("\nCommandes internes disponibles :")
    print("  help   : Affiche cette aide")
    print("  cd     : Change le répertoire courant")
    print("  if     : Exécute un bloc conditionnel personnalisé")
    print("  for    : Exécute une boucle personnalisée")
    print("  unset  : Supprime une variable d’environnement définie")
    print("  exit   : Quitte le shell\n")
    shell_state["?"] = 0
