# Self-Question C Implementations — CN Lab

Extracted from CN Lab record self-questions (pages 17, 23, 26, 14).
All 26 programs compile cleanly with gcc (Ubuntu 13.3.0), tested with:
  gcc -Wall -Wextra <file>.c -o <out> [-lpthread]

## Folder map

page17/  (TCP basics)
  q1_echo_server.c / q1_echo_client.c           - Echo server (Self-Q1)
  q2_palindrome_server.c / q2_palindrome_client.c - Palindrome checker (Self-Q2)
  q3_auth_menu_server.c / q3_auth_menu_client.c  - Authenticated multi-service menu: upload/download/date-time/sysinfo (Self-Q3)
  q4_student_db_server.c / q4_student_db_client.c - Student Result Management System (Self-Q4)

page23/  (Iterative TCP servers - one client fully served before next)
  q1_railway_server.c / q1_railway_client.c   - Railway Reservation System (Self-Q1)
  q2_hospital_server.c / q2_hospital_client.c - Hospital Appointment Management (Self-Q2)
  q3_movie_server.c / q3_movie_client.c       - Movie Ticket Booking System (Self-Q3)
  q4_library_server.c / q4_library_client.c   - College Library Management System (Self-Q4)

page26/  (Concurrent/multithreaded TCP servers, pthread + mutex)
  q1_exam_mgmt_server.c / q1_exam_mgmt_client.c         - Online Exam Management (Self-Q1)
  q2_hotel_reservation_server.c / q2_hotel_reservation_client.c - Hotel Reservation, mutex-guarded booking (Self-Q2)
  q3_file_share_server.c / q3_file_share_client.c       - Concurrent File Sharing Server (Self-Q3)
  q4_chat_broadcast_server.c / q4_chat_broadcast_client.c - Chat Notification broadcast server (Self-Q4)

page14/  (Sliding window protocol simulations, no sockets)
  q1_go_back_n.c        - Go-Back-N ARQ simulation (Self-Q1)
  q2_selective_repeat.c - Selective Repeat ARQ simulation (Self-Q2)

## Build & run examples

Simple TCP pair:
  gcc page17/q1_echo_server.c -o echo_server && ./echo_server
  gcc page17/q1_echo_client.c -o echo_client && ./echo_client   # in a second terminal

Concurrent (needs pthread):
  gcc page26/q1_exam_mgmt_server.c -o exam_server -lpthread && ./exam_server
  gcc page26/q1_exam_mgmt_client.c -o exam_client && ./exam_client

Standalone simulation (no server/client, no sockets):
  gcc page14/q1_go_back_n.c -o gbn && ./gbn

All servers listen on 127.0.0.1 on distinct ports (5000-5003, 6001-6004, 7001-7004)
so several can run simultaneously without conflict. Each server persists its data
to plain-text .dat files and appends a timestamped .log file in its working directory.
