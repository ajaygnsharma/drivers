import tkinter as tk

window = tk.Tk();

def calculate():
    degF = entry_temperature.get();
    degC = (5/9)*(float(degF) - 32);
    lblResult["text"] = f"{round(degC,2)} \N{DEGREE CELSIUS}";

window.rowconfigure(0, minsize=100);
window.columnconfigure([0, 1, 2], minsize=50);

frame1 = tk.Frame(master=window);
entry_temperature = tk.Entry(master=frame1, width=10);
lblTemp = tk.Label(master=frame1, text="\N{DEGREE FAHRENHEIT}");


entry_temperature.grid(row=0, column=0, stick="e");
lblTemp.grid(row=0, column=1, sticky="w");


btnConvert = tk.Button(master=window, text="\N{RIGHTWARDS BLACK ARROW}", command=calculate);
lblResult  = tk.Label(master=window, text="\N{DEGREE CELSIUS}");


frame1.grid(row=0,column=0, padx=10);
btnConvert.grid(row=0, column=1, pady=10);
lblResult.grid(row=0, column=2, padx=10);

window.mainloop();



