#
# Simple Makefile for main.c
#

CC=g++
CFLAGS=-std=c++11 -Wall -lpthread

all: pgrep

pgrep: pgrep.c
	$(CC) $(CFLAGS) pgrep.c -o pgrep 

run: 
	./pgrep