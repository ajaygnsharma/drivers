import tkinter as tk

window = tk.Tk();
frame  = tk.Frame();
frame.pack();

label = tk.Label(master=frame, text="Hi there");
label.pack();

def handle_text_entry(event):
    print(event.char);


entry1 = tk.Entry(master=frame, text="Foo");
entry1.pack();
entry1.insert(0, "Foo");

entry1.bind("<Key>", handle_text_entry);

def handle_click(event):
    print("Button clicked");

button = tk.Button(text="Calculate");
button.bind("<Button-1>", handle_click); # Button-1 is left click. -3 is right click
button.pack();

window.mainloop();

