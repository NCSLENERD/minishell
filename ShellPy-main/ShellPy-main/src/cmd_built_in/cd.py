import os

def cd(path = None):

    try :
        if path is None or path.strip() == "":
            path = os.path.expanduser("~")
            return 0
        os.chdir(path)
        return 0
    except FileNotFoundError:
        print(f"cd : {path}: Aucun fichier ou dossier de ce type")
        return 1
    except PermissionError:
        print(f"cd : {path}: Permission refusée")
        return 1
    except NotADirectoryError:
        print(f"cd : {path}: Ce n'est pas un répertoire")
        return 1