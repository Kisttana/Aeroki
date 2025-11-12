import tkinter as tk
from tkinter import filedialog, messagebox

def save_function():
    name = func_name.get().strip()
    code = text_box.get("1.0", tk.END).strip()
    if not name:
        messagebox.showerror("Error", "กรุณาใส่ชื่อฟังก์ชัน")
        return
    if not code:
        messagebox.showerror("Error", "ไม่มีโค้ดในฟังก์ชัน")
        return

    content = f"ฟังก์ชัน {name}\n{code}\nจบฟังก์ชัน\n"
    path = filedialog.asksaveasfilename(
        defaultextension=".aerofunc",
        filetypes=[("Aeroki Function", "*.aerofunc")],
        initialfile=f"{name}.aerofunc"
    )
    if path:
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)
        messagebox.showinfo("Saved", f"ฟังก์ชัน '{name}' ถูกบันทึกแล้ว")

root = tk.Tk()
root.title("Aeroki Function Creator")
root.geometry("600x400")

tk.Label(root, text="ชื่อฟังก์ชัน:", font=("Consolas", 12)).pack(anchor="w", padx=10, pady=5)
func_name = tk.Entry(root, font=("Consolas", 12))
func_name.pack(fill="x", padx=10)

tk.Label(root, text="โค้ดในฟังก์ชัน:", font=("Consolas", 12)).pack(anchor="w", padx=10, pady=5)
text_box = tk.Text(root, font=("Consolas", 12), height=15)
text_box.pack(fill="both", expand=True, padx=10)

tk.Button(root, text="บันทึกฟังก์ชัน", command=save_function).pack(pady=10)

root.mainloop()
