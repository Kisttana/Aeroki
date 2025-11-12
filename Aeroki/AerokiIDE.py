import subprocess
import tkinter as tk
from tkinter import filedialog, scrolledtext, simpledialog
import tempfile
import threading
import os

class AerokiIDE:
    def __init__(self, root):
        self.root = root
        self.root.title("Aeroki IDE")
        self.root.geometry("800x600")

        # --- Menu ---
        menu = tk.Menu(root)
        root.config(menu=menu)

        file_menu = tk.Menu(menu, tearoff=False)
        file_menu.add_command(label="Open", command=self.open_file)
        file_menu.add_command(label="Save", command=self.save_file)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=root.quit)
        menu.add_cascade(label="File", menu=file_menu)

        # --- Text Editor ---
        self.editor = scrolledtext.ScrolledText(root, wrap=tk.WORD, font=("Consolas", 12))
        self.editor.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        # --- Run Button ---
        self.run_button = tk.Button(
            root, text="▶ Run", command=self.run_aeroki,
            bg="#4CAF50", fg="white", font=("Consolas", 12)
        )
        self.run_button.pack(pady=5)

        # --- Output Box ---
        self.output_box = scrolledtext.ScrolledText(
            root, wrap=tk.WORD, height=10, font=("Consolas", 11),
            bg="#111", fg="#0f0"
        )
        self.output_box.pack(fill=tk.BOTH, expand=False, padx=10, pady=10)

        self.filename = None

    def open_file(self):
        self.filename = filedialog.askopenfilename(
            filetypes=[("Aeroki files", "*.aero"), ("All files", "*.*")]
        )
        if not self.filename:
            return
        with open(self.filename, "r", encoding="utf-8") as f:
            code = f.read()
            self.editor.delete("1.0", tk.END)
            self.editor.insert(tk.END, code)

    def save_file(self):
        if not self.filename:
            self.filename = filedialog.asksaveasfilename(
                defaultextension=".aero",
                filetypes=[("Aeroki files", "*.aero")]
            )
        if not self.filename:
            return
        with open(self.filename, "w", encoding="utf-8") as f:
            f.write(self.editor.get("1.0", tk.END))

    # --- Fixed non-freezing Run system ---
    def run_aeroki(self):
        self.output_box.delete("1.0", tk.END)
        code = self.editor.get("1.0", tk.END)

        # --- Save temporary file ---
        temp_dir = tempfile.gettempdir()
        temp_file = os.path.join(temp_dir, "temp.aero")

        try:
            with open(temp_file, "w", encoding="utf-8") as f:
                f.write(code)
        except PermissionError:
            self.output_box.insert(tk.END, f"Permission denied when writing to: {temp_file}\n")
            return

        try:
            exe_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "aeroki.exe")
            if not os.path.exists(exe_path):
                raise FileNotFoundError

            cmd = [exe_path, temp_file] if subprocess.os.name == "nt" else ["./aeroki", temp_file]

            # --- Run process interactively ---
            process = subprocess.Popen(
                cmd,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                encoding="utf-8",
                bufsize=1
            )

            def read_output():
                for line in process.stdout:
                    # Detect Aeroki input requests
                    if line.startswith("__AEROKI_INPUT_REQUEST__"):
                        prompt = line.strip().split("__AEROKI_INPUT_REQUEST__")[1]
                        value = simpledialog.askstring("Input", f"กรอกค่า {prompt}:")
                        if value is None:
                            value = "0"
                        process.stdin.write(value + "\n")
                        process.stdin.flush()
                    else:
                        # Normal output
                        self.output_box.insert(tk.END, line)
                        self.output_box.see(tk.END)
                process.wait()

            # Run in background thread to keep GUI responsive
            threading.Thread(target=read_output, daemon=True).start()

        except FileNotFoundError:
            self.output_box.insert(
                tk.END,
                "Aeroki compiler not found!\nMake sure 'aeroki.exe' exists in the same folder."
            )
        finally:
            try:
                os.remove(temp_file)
            except Exception:
                pass


# --- Run GUI ---
if __name__ == "__main__":
    root = tk.Tk()
    app = AerokiIDE(root)
    root.mainloop()
