#!/usr/bin/python

import tkinter
import tkinter.messagebox
import socket
from struct import *
from enum import Enum

class ServerErrors(Enum):
    SERR_SUCCESS = 0
    SERR_ADDERROR = 1
    SERR_GETERROR = 2

class ServerCommands(Enum):
    MOVIE_ADD = 1
    MOVIE_DEL = 2
    MOVIE_LIST = 3
    MOVIE_GET = 4

HOST = '10.0.0.156'       # Movie server host ip
PORT = 8888     # Port used by movie server

genres = ['Action', 'Comedy', 'Horror', 'SciFi', 'Romantic Comedy']
server_genres = ['action', 'comedy', 'horror', 'scifi', 'romcom']
packet_hdr_size = 8

def SendHeader(s, cmd_type, packet_len):
    s.send(pack('=II', cmd_type, packet_len))

def RecvHeader(s):
    recv_packet = s.recv(packet_hdr_size)
    return unpack('=II', recv_packet)

def ServerConnect():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    return s

def ServerDisconnect(s):
    s.shutdown(socket.SHUT_RDWR)
    s.close()

def AddCallBack():
    s = ServerConnect()
    genre_name = server_genres[genres.index(genre_variable.get())]
    movie_name = movie_entry.get()

    genre_len = len(genre_name)
    movie_len = len(movie_name)

    packet_len = packet_hdr_size + 8 + genre_len + movie_len
    SendHeader(s, ServerCommands.MOVIE_ADD.value, packet_len)

    pack_string = '=I' + str(genre_len) + 'sI' + str(movie_len) + 's'
    packed = pack(pack_string, genre_len, genre_name.encode(), movie_len, movie_name.encode())
    s.send(packed)

    (ret_status, packet_len) = RecvHeader(s)
    if (ret_status == ServerErrors.SERR_ADDERROR.value):
        tkinter.messagebox.showinfo( "Movie Add Result", "Error in adding movie.")
        return
    else:
        tkinter.messagebox.showinfo( "Movie Add Result", "Successfully added movie!")
    movie_entry.delete(0, 'end')
    ServerDisconnect(s)

def GetCallBack():
    s = ServerConnect()
    genre_name = server_genres[genres.index(genre_variable.get())]

    genre_len = len(genre_name)

    packet_len = packet_hdr_size + 4 + genre_len
    SendHeader(s, ServerCommands.MOVIE_GET.value, packet_len)
    pack_string = '=I' + str(genre_len) + 's'
    packed = pack(pack_string, genre_len, genre_name.encode())

    s.send(packed)

    (ret_status, packet_len) = RecvHeader(s)

    if (ret_status == ServerErrors.SERR_GETERROR.value):
        tkinter.messagebox.showinfo( "Movie Add Result", "Could not retrieve a movie.")
        return

    movie_ret_size = s.recv(4)[0]

    (movie_ret_bytes) = s.recv(movie_ret_size)

    user_str = "Movie retrieved successfully!\n" + movie_ret_bytes.decode()
    tkinter.messagebox.showinfo( "Movie Add Result", user_str)

    ServerDisconnect(s)
    pass

top = tkinter.Tk()
top.title('Movie Generator')

genre_label = tkinter.Label(top, text="Movie Genre")
genre_label.grid(row = 0, column = 0)
genre_variable = tkinter.StringVar(top)
genre_variable.set(genres[0])
genre_list = tkinter.OptionMenu(top, genre_variable, *genres)
genre_list.grid(row = 0, column = 1)

movie_label = tkinter.Label(top, text="Movie Name")
movie_label.grid(row = 1, column = 0)
movie_entry = tkinter.Entry(top, bd =5)
movie_entry.grid(row = 1, column = 1)

add_button = tkinter.Button(top, text ="Add Movie", command = AddCallBack)
add_button.grid(row = 1, column = 2)

get_button = tkinter.Button(top, text ="Get Movie", command = GetCallBack)
get_button.grid(row = 3, column = 1)

# Code to add widgets will go here...
top.mainloop()
