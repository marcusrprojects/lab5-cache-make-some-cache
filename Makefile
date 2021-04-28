#
# Makefile for Lab 5
# 
CC = gcc
CFLAGS = -g -Wall -Werror -std=c99

cachesim: cachesim.c
	$(CC) $(CFLAGS) -o cachesim cachesim.c -lm

clean:
	rm -f cachesim trace.all trace.f* .cachesim_results .marker
