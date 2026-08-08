import os

def print_screen():
    os.system('clear')

    PINK = "\033[95m"
    RESET = "\033[0m"

    banner = r"""
     ________  ___  ___  _______   ___       ___               ________  ___    ___ 
    |\   ____\|\  \|\  \|\  ___ \ |\  \     |\  \             |\   __  \|\  \  /  /|
    \ \  \___|\ \  \\\  \ \   __/|\ \  \    \ \  \            \ \  \|\  \ \  \/  / /
     \ \_____  \ \   __  \ \  \_|/_\ \  \    \ \  \            \ \   ____\ \    / / 
      \|____|\  \ \  \ \  \ \  \_|\ \ \  \____\ \  \____        \ \  \___|\/  /  /  
        ____\_\  \ \__\ \__\ \_______\ \_______\ \_______\       \ \__\ __/  / /    
       |\_________\|__|\|__|\|_______|\|_______|\|_______|        \|__||\___/ /     
       \|_________|                                                    \|___|/      
    """

    terminal_width = os.get_terminal_size().columns
    for line in banner.splitlines():
        print(PINK + line.center(terminal_width) + RESET)

    signature = "by Laghsal Riwan & Mehnana Ilyes"
    print(PINK + signature.center(terminal_width) + RESET)
