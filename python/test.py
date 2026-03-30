import baba
import sys

print("Baba version:", baba.__version__)
print("Platform:", sys.platform)

try:
    app = baba.App()
    print("App created successfully")

    win = baba.Window(app, "Test", 300, 200)
    print("Window created successfully")

    win.show()
    print("Window shown")

    print("Starting run loop - close window to exit")
    app.run()
    print("Run loop finished")
except Exception as e:
    print(f"Error: {e}")
    import traceback

    traceback.print_exc()
