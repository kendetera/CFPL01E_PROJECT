#include "parking.h"

int main()
{
    ParkingSpot parkingLot[MAX_PARKING_SPOTS]; // Array of parking spots
    int choice;
    sqlite3* db;
    int rc;

    // Initialize parking spots
    initializeParkingSpots(parkingLot, MAX_PARKING_SPOTS);

    // Open SQLite database
    rc = sqlite3_open("ParkingHistory.db", &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n");
    }

    // Create table if it doesn't exist
    createTable(db);

    // Main loop
    while (1)
    {
        showMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            parkVehicle(parkingLot, MAX_PARKING_SPOTS, db);
            break;
        case 2:
            removeVehicle(parkingLot, MAX_PARKING_SPOTS, db);
            break;
        case 3:
            displayParkingRates();
            break;
        case 4:
            displayParkingReport(db);
            break;
        case 5:
        {
            char vehicleNumber[15];
            printf("Enter vehicle number to remove history: ");
            scanf("%s", vehicleNumber);
            removeParkingHistory(db, vehicleNumber);
        }
        break;
        case 6:
            removeAllParkingHistory(db);
            break;
        case 7:
            sqlite3_close(db);
            exit(0);
        default:
            printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}