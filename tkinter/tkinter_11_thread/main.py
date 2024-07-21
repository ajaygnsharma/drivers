# Import Module
import tkinter as tk
import time
from threading import *

# Create Object
root = tk.Tk()

# Set geometry
root.geometry("400x400")

# Create text widget and specify size.
T = tk.Text(root, height = 5, width = 52);
T.pack();

# use threading
def threading():
    # Call work function
    t1 = Thread(target=work);
    t1.start();

# work function
def work():
    print("sleep time start")

    for i in range(10):
        print(i);
        T.insert(tk.END, str(i));
        time.sleep(1)

    print("sleep time stop")


# Create Button
tk.Button(root, text="Click Me", command=threading).pack()


# Execute Tkinter
root.mainloop()