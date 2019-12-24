from socket import AF_INET, socket, SOCK_STREAM
from threading import Thread
import Tkinter as Tk

# Socket with given AWS parameters.
HOST = "35.162.177.130"
PORT = 3002
BUFFER_SIZE = 1024
ADDR = (HOST, PORT)

client_socket = socket(AF_INET, SOCK_STREAM)
client_socket.connect(ADDR)
# This function must be threaded.
def receive():
    while True:
        try:
            msg = client_socket.recv(BUFFER_SIZE).decode("utf8")
            msg_list.insert(Tk.END, msg)
            msg_list.see(Tk.END)
        except OSError:
            break


def send(event=None):  # Binders pass the event.
    msg = my_msg.get()
    my_msg.set("")      # Clears text field.
    global current_room
    if msg == "{quit}":
        client_socket.send(bytes(my_username.get() + " has closed OS Messenger App!", "utf8"))
        client_socket.close()
        top.quit()
        return
    client_socket.send(bytes(my_username.get() + ": " + msg))


# Send quit message to the server.
def on_closing(event=None):
    # Send server quit message.
    my_msg.set("{quit}")
    send()


def change_room():
    global current_room
    current_room = ((chatRoomSelected.get()).split(' '))[2]
    client_socket.send(bytes("/" + current_room))
    msg_list.delete(0, Tk.END)
    msg_list.insert(Tk.END, "You are now in room " + str(current_room))
    msg_list.see(Tk.END)




top = Tk.Tk()
top.title("OS Messenger App")
#top.resizable(width=False, height=False)  


messages_frame = Tk.Frame(top)
my_msg = Tk.StringVar()  # For the messages to be sent.
my_msg.set("")
my_username = Tk.StringVar()
my_username.set("")

scrollbar = Tk.Scrollbar(messages_frame)  # To see through previous messages.
# Messages container.
msg_list = Tk.Listbox(messages_frame, height=30, width=100, yscrollcommand=scrollbar.set)
scrollbar.pack(side=Tk.RIGHT, fill=Tk.Y)
msg_list.pack(side=Tk.LEFT, fill=Tk.BOTH)
msg_list.pack()
messages_frame.pack()

username_label = Tk.Label(top, text="Enter username: ")
username_label.pack()
username_field = Tk.Entry(top, textvariable=my_username)
username_field.pack()

message_label = Tk.Label(top, text="Enter message: ")
message_label.pack()
entry_field = Tk.Entry(top, textvariable=my_msg, width=50)
entry_field.bind("<Return>", send)
entry_field.pack()
send_button = Tk.Button(top, text="Send", command=send)
send_button.pack()


top.protocol("WM_DELETE_WINDOW", on_closing)
top.mainloop()


# Server response of number of rooms available and generate drop down list.
first_msg = client_socket.recv(BUFFER_SIZE).decode("utf8")
number_of_rooms = int(first_msg)
chatRoomSelected = Tk.StringVar(top)
chatRoomSelected.set("List Of Chat Rooms")
rooms_list = []
for i in range(number_of_rooms):
    rooms_list.append("Chat Room " + str(i + 1))

chat_rooms = Tk.OptionMenu(top, chatRoomSelected, *rooms_list)
chat_rooms.pack()
change_button = Tk.Button(top, text="Change Room", command=change_room)
change_button.pack()

receive_thread = Thread(target=receive)
receive_thread.start()
# The client can't resize the window.
