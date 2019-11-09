# CSV Data Sorting Using Sockets (Client Server Programming)

## Setup Instructions
- Clone this repository (```git clone https://github.com/harshagrwl/CSV-Data-Sorting.git```)
- Change the directory to the Cloned Repository (```cd CSV-Data-Sorting```)
- Compile the Server (```gcc sorter_server.c -lpthread -o ser```)
- Compile the Client (```gcc sorter_client.c -lpthread -o cli```)
- Open two terminal windows.
- In the first one, run the Server (```./ser -p 5567```)
- In the second one, run the Client (```./cli -c <column to sort>  -h 127.0.0.1 -p 5576```)
- A new CSV file will be generated with the sorted values of the specified coloumn.

## Contribution
- Pull Requests and suggestions are welcome.
