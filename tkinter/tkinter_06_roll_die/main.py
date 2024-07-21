import tkinter as tk
import random

window = tk.Tk();
window.columnconfigure(0, minsize=150);
window.rowconfigure([0, 1], minsize=50);

def roll():
    lbl["text"] = str(random.randint(1,6));

btn = tk.Button(text="Roll", command=roll);
lbl = tk.Label(text="0");

btn.grid(row=0, column=0, sticky="nsew");
lbl.grid(row=1, column=0);

window.mainloop();



