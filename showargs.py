import gdb

class ShowArgsCommand(gdb.Command):

    def __init__(self):
        super(ShowArgsCommand, self).__init__("showargs", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        frame = gdb.selected_frame()
        block = frame.block()

        print("Argumenti funkcije:")
        for symbol in block:
            if symbol.is_argument:
                try:
                    val = symbol.value(frame)
                    print(f"  {symbol.name}: {val}")
                except Exception as e:
                    print(f"  {symbol.name}: <ne moze se procitati>")

ShowArgsCommand()

