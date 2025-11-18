import subprocess
import threading
import queue
import tkinter as tk
from tkinter import filedialog, scrolledtext, simpledialog
import tempfile
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

    def run_aeroki(self):
        self.output_box.delete("1.0", tk.END)
        code = self.editor.get("1.0", tk.END)

        # --- Save temporary file safely in system temp folder ---
        temp_dir = tempfile.gettempdir()
        temp_file = os.path.join(temp_dir, "temp.aero")

        try:
            with open(temp_file, "w", encoding="utf-8") as f:
                f.write(code)
        except PermissionError:
            self.output_box.insert(tk.END, f"Permission denied when writing to: {temp_file}\n")
            return

        try:
            # Locate aeroki.exe in same folder as this script
            exe_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "aeroki.exe")
            if not os.path.exists(exe_path):
                raise FileNotFoundError

            cmd = [exe_path, temp_file] if subprocess.os.name == "nt" else ["./aeroki", temp_file]

            # If a previous process is running, terminate it
            if hasattr(self, 'proc') and self.proc and self.proc.poll() is None:
                try:
                    self.proc.kill()
                except Exception:
                    pass

            # Start Aeroki as a subprocess with pipes so we can interactively
            # forward input prompts back to the GUI without blocking.
            self.proc = subprocess.Popen(
                cmd,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                encoding='utf-8',
                bufsize=1
            )

            prompt_queue = queue.Queue()

            def stdout_reader():
                # Read one character at a time to detect prompts that have no newline
                buf = ''
                while True:
                    ch = self.proc.stdout.read(1)
                    if ch == '':
                        break
                    buf += ch
                    # Append to output box on main thread
                    self.root.after(0, lambda c=ch: self.output_box.insert(tk.END, c))
                    # Keep buffer short
                    if len(buf) > 1024:
                        buf = buf[-512:]
                    # Detect prompt prefix "กรอกค่า"
                    if 'กรอกค่า' in buf:
                        # Try to capture until ':' which the C prompt prints
                        if ':' in buf:
                            # extract variable name between 'กรอกค่า ' and ':'
                            try:
                                start = buf.rfind('กรอกค่า')
                                after = buf[start:]
                                # find colon in after
                                colon = after.find(':')
                                prompt_text = after[:colon]
                                # prompt_text looks like 'กรอกค่า name'
                                parts = prompt_text.split()
                                varname = parts[1] if len(parts) > 1 else ''
                            except Exception:
                                varname = ''
                            prompt_queue.put(varname)
                            buf = ''

            def stderr_reader():
                for line in self.proc.stderr:
                    self.root.after(0, lambda l=line: self.output_box.insert(tk.END, l))

            t_out = threading.Thread(target=stdout_reader, daemon=True)
            t_err = threading.Thread(target=stderr_reader, daemon=True)
            t_out.start()
            t_err.start()

            # Poll queue on main thread to display input dialog and send input
            def poll_prompt():
                try:
                    varname = prompt_queue.get_nowait()
                except queue.Empty:
                    if self.proc.poll() is None:
                        self.root.after(100, poll_prompt)
                    return

                # Show input dialog on main thread
                prompt_label = f"กรอกค่า {varname}:" if varname else "กรอกค่า:"
                answer = simpledialog.askstring("Input", prompt_label, parent=self.root)
                if answer is None:
                    answer = ''
                # send answer with newline to subprocess stdin
                try:
                    if self.proc and self.proc.stdin:
                        self.proc.stdin.write(answer + '\n')
                        self.proc.stdin.flush()
                except Exception:
                    pass

                # continue polling
                if self.proc.poll() is None:
                    self.root.after(100, poll_prompt)

            # start polling
            self.root.after(100, poll_prompt)

            # Wait for process to finish in a background thread so GUI stays responsive
            def waiter():
                self.proc.wait()
                # ensure any remaining stdout/stderr are read before finishing
                try:
                    remaining = self.proc.stdout.read()
                    if remaining:
                        self.root.after(0, lambda r=remaining: self.output_box.insert(tk.END, r))
                except Exception:
                    pass
                self.root.after(0, lambda: self.output_box.insert(tk.END, f"\n[Process exited with {self.proc.returncode}]\n"))

            threading.Thread(target=waiter, daemon=True).start()
        except FileNotFoundError:
            self.output_box.insert(
                tk.END,
                "Aeroki compiler not found!\nMake sure 'aeroki.exe' exists in the same folder."
            )
        finally:
            # --- Clean up temporary file ---
            try:
                os.remove(temp_file)
            except Exception:
                pass

# --- Run the GUI ---
if __name__ == "__main__":
    root = tk.Tk()
    app = AerokiIDE(root)
    root.mainloop()
