import tkinter as tk

window = tk.Tk();

def dec():
    value = int(lbl["text"]);
    lbl["text"] = f"{value - 1}";

def inc():
    value = int(lbl["text"]);
    lbl["text"] = f"{value + 1}";

window.rowconfigure(0, minsize=50, weight=1);
window.columnconfigure([0, 1, 2], minsize=50, weight=1);

btn_dec = tk.Button(master=window, text="-", command=dec);
btn_dec.grid(row=0, column=0, sticky="nsew");

lbl = tk.Label(master=window, text="0");
lbl.grid(row=0, column=1);

btn_inc = tk.Button(master=window, text="+", command=inc);
btn_inc.grid(row=0, column=2, sticky="nsew");

window.mainloop();




