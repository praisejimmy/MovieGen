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

HOST = ''       # Movie server host ip
PORT = 8888     # Port used by movie server

genres = ['Action', 'Comedy', 'Horror', 'SciFi', 'Romantic Comedy']
server_genres = ['action', 'comedy', 'horror', 'scici', 'romcom']
packet_hdr_size = 5

def SendHeader(s, cmd_type, packet_len):
    s.sendall(pack('BI', cmd_type, packet_len))

def RecvHeader(s):
    recv_packet = s.recv(packet_hdr_size)
    return unpack('BI', recv_packet)

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
    SendHeader(s, MOVIE_ADD, packet_len)

    pack_string = 'I' + str(genre_len) + 'sI' + str(movie_len) + 's'
    packed = pack(pack_string, genre_len, genre_name.encode(), movie_len, movie_name.encode())

    s.sendall(packed)

    (ret_status, packet_len) = RecvHeader()
    if (ret_status == SERR_ADDERROR):
        tkinter.messagebox.showinfo( "Movie Add Result", "Error in adding movie.")
    else:
        tkinter.messagebox.showinfo( "Movie Add Result", "Successfully added movie!")
    ServerDisconnect(s)

def GetCallBack():
    s = ServerConnect()
    genre_name = server_genres[genres.index(genre_variable.get())]

    genre_len = len(genre_name)

    packet_len = packet_hdr_size + 4 + genre_len
    SendHeader(s, MOVIE_GET, packet_len)
    pack_string = 'I' + str(genre_len) + 's'
    packed = pack(pack_string, genre_len, genre_name.encode(), movie_len, movie_name.encode())

    s.sendall(packed)

    (ret_status, packet_len) = RecvHeader()

    if (ret_status == SERR_GETERROR):
        tkinter.messagebox.showinfo( "Movie Add Result", "Could not retrieve a movie.")
        return

    movie_ret_size_bytes = s.recv(4)
    movie_ret_size = unpack('I', movie_ret_size)

    movie_ret_bytes = s.recv(movie_ret_size)

    user_str = "Movie retrieved successfully!\n" + movie_ret_bytes.decode()
    tkinter.messagebox.showinfo( "Movie Add Result", user_str)

    ServerDisconnect(s)
    pass

top = tkinter.Tk()
top.title('Movie Generator')

genre_label = tkinter.Label(top, text="Movie Genre")
genre_label.pack( side = tkinter.LEFT)
genre_variable = tkinter.StringVar(top)
genre_variable.set(genres[0])
genre_list = tkinter.OptionMenu(top, genre_variable, *genres)
genre_list.pack()

movie_label = tkinter.Label(top, text="Movie Name")
movie_label.pack( side = tkinter.LEFT)
movie_entry = tkinter.Entry(top, bd =5)
movie_entry.pack(side = tkinter.LEFT)

add_button = tkinter.Button(top, text ="Add Movie", command = AddCallBack)
add_button.pack()

get_button = tkinter.Button(top, text ="Get Movie", command = GetCallBack)
get_button.pack()

# Code to add widgets will go here...
top.mainloop()
