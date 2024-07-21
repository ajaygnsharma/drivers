#Using classic tk window technique
import tkinter as tk

window = tk.Tk();
greeting = tk.Label(text="Hello, Tkinter");
greeting.pack();

name = tk.Entry(width=10);
name.pack();
window.mainloop();


