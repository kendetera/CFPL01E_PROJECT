#include "parking.h"

// Declare the callback function
int callback(void* table, int argc, char** argv, char** azColName);

// Function to show the menu
void showMenu()
{
    printf("\nParking System Menu:\n");
    printf("1. Park Vehicle\n");
    printf("2. Remove Vehicle\n");
    printf("3. Display Parking Rates\n");
    printf("4. Display Parking Report\n");
    printf("5. Remove Parking History by Vehicle Number\n");
    printf("6. Remove All Parking History\n");
    printf("7. Exit\n");
}

// Function to initialize parking spots
void initializeParkingSpots(ParkingSpot parkingLot[], int totalSpots)
{
    for (int i = 0; i < totalSpots; i++)
    {
        parkingLot[i].spotId = i + 1;
        parkingLot[i].isOccupied = 0;
        memset(parkingLot[i].vehicle.vehicleNumber, 0, VEHICLE_NUMBER_SIZE);
    }
}

// Function to park a vehicle
void parkVehicle(ParkingSpot parkingLot[], int totalSpots, sqlite3* db) {
    int spotFound = 0;
    double paymentAmount, change;

    // Seed the random number generator
    srand(time(NULL));

    for (int i = 0; i < totalSpots; i++) {
        if (!parkingLot[i].isOccupied) {
            parkingLot[i].isOccupied = 1;
            printf("Enter vehicle number: ");
            if (scanf("%14s", parkingLot[i].vehicle.vehicleNumber) != 1) { // Handle input
                printf("Invalid input.\n");
                return;
            }
            parkingLot[i].vehicle.entryTime = time(NULL);
            parkingLot[i].vehicle.isParked = 1;

            // Generate a random ticket number
			parkingLot[i].vehicle.ticketNumber = 1000 + rand() % 9000; // Random 4-digit number

            // Ask for the amount paid by the user
            printf("Enter Payment Amount: ");
            if (scanf("%lf", &paymentAmount) != 1) { // Handle input
                printf("Invalid input.\n");
                return;
            }

            // Calculate the change
            change = paymentAmount - FIXED_PARKING_RATE;
            if (change < 0) {
				system("clear");
                printf("Insufficient amount. Parking fee is P%d.\n", FIXED_PARKING_RATE);
                parkingLot[i].isOccupied = 0;
                return;
            }

            system("clear");
            printf("Vehicle %s parked at spot %d.\n", parkingLot[i].vehicle.vehicleNumber, parkingLot[i].spotId);
            printf("Ticket Number: %d\n", parkingLot[i].vehicle.ticketNumber);
            printf("Amount Paid: P%.2f\n", paymentAmount);
            printf("Change: P%.2f\n", change);

            // Insert parking history into the database
            insertParkingHistory(db, parkingLot[i].vehicle.vehicleNumber, parkingLot[i].spotId, parkingLot[i].vehicle.entryTime);
            spotFound = 1;
            break;
        }
    }

    if (!spotFound) {
        system("clear");
        printf("No free parking spots available.\n");
    }
}

// Function to remove a vehicle
void removeVehicle(ParkingSpot parkingLot[], int totalSpots, sqlite3* db)
{
    char licensePlate[VEHICLE_NUMBER_SIZE] = { 0 }; // Ensure zero-terminated
    int ticketNumber; // Ticket number
    printf("Enter vehicle license plate to exit: ");
    if (scanf("%14s", licensePlate) != 1) 
    {
        printf("Invalid input.\n");
        return;
    }

    int found = 0;
    for (int i = 0; i < totalSpots; i++)
    {
        if (parkingLot[i].isOccupied && strcmp(parkingLot[i].vehicle.vehicleNumber, licensePlate) == 0)
        {
            int attempts = 0;
            int correctTicket = 0;
            while (attempts < 3)
            {
                printf("Enter ticket number: ");
                if (scanf("%d", &ticketNumber) != 1) // Handle input
                {
                    printf("Invalid input.\n");
                    return;
                }

                if (parkingLot[i].vehicle.ticketNumber == ticketNumber)
                {
                    correctTicket = 1;
                    break;
                }
                else
                {
                    printf("Incorrect ticket number. Try again.\n");
                    attempts++;
                }
            }

            time_t exitTime = time(NULL); // Get exit time
            double fee = calculateParkingFee(parkingLot[i].vehicle.entryTime, exitTime);

            // Apply lost ticket fee if the ticket number was incorrect after 3 attempts
            if (!correctTicket)
            {
                fee += LOST_TICKET_FEE;
                system("clear");
                printf("Incorrect ticket number entered 3 times. Lost ticket fee applied.\n");
            }

            // Update parking history in the database
            updateParkingHistory(db, parkingLot[i].vehicle.vehicleNumber, exitTime, fee);

			system("clear");
            printf("Vehicle with license plate %s exited from spot %d.\n", licensePlate, parkingLot[i].spotId);
            printf("Parking duration: %.2f hours. Fee: P%.2f\n",
                difftime(exitTime, parkingLot[i].vehicle.entryTime) / 3600.0, fee);

            parkingLot[i].isOccupied = 0; // Mark spot as available
            parkingLot[i].vehicle.isParked = 0;
            found = 1;
            break;
        }
    }

    if (!found)
    {
        system("clear");
        printf("Vehicle with license plate %s not found.\n", licensePlate);
    }
}


// Function to calculate parking fee
double calculateParkingFee(time_t entryTime, time_t exitTime)
{
    double hours = difftime(exitTime, entryTime) / 3600.0;

    if (hours < 24)
    {
        return FIXED_PARKING_RATE;
    }
    else
    {
        return FIXED_PARKING_RATE + OVERNIGHT_FEE;
    }
}

// Function to remove parking history by vehicle number
void removeParkingHistory(sqlite3* db, const char* vehicleNumber)
{
    char* errMsg = 0;
    char sql[256];
    snprintf(sql, sizeof(sql), "DELETE FROM ParkingHistory WHERE VehicleNumber = '%s';", vehicleNumber);

    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    else
    {
		system("clear");
        printf("Parking history for vehicle %s removed successfully.\n", vehicleNumber);
    }
}

// Function to remove all parking history
void removeAllParkingHistory(sqlite3* db)
{
    char* errMsg = 0;
    const char* sql = "DELETE FROM ParkingHistory;";

    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    else
    {
		system("clear");
        printf("All parking history removed successfully.\n");
    }
}

// Function to display parking rates
void displayParkingRates()
{   
	system("clear");
    printf("Parking Rates:\n");
    printf("Fixed Rate: P%d\n", FIXED_PARKING_RATE);
    printf("Overnight Fee: P%d\n", OVERNIGHT_FEE);
    printf("Lost Ticket Fee: P%d\n", LOST_TICKET_FEE);
}

// Function to display parking report
void displayParkingReport(sqlite3* db)
{
    char* errMsg = 0;
    const char* sql = "SELECT * FROM ParkingHistory;";
	system("clear");

    ft_table_t* table = ft_create_table();
    ft_set_border_style(table, FT_BASIC_STYLE);
    ft_set_cell_prop(table, FT_ANY_ROW, FT_ANY_COLUMN, FT_CPROP_TEXT_ALIGN, FT_ALIGNED_CENTER);
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);

    // Add table headers
    ft_write_ln(table, "Vehicle Number", "Spot Number", "Entry Time", "Exit Time", "Parking Fee");

    int rc = sqlite3_exec(db, sql, callback, table, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    else
    {
        printf("%s\n", ft_to_string(table));
    }

    ft_destroy_table(table);
}

// Callback function for parking report
int callback(void* table, int argc, char** argv, char** azColName)
{
    ft_table_t* ft_table = (ft_table_t*)table;
    ft_write_ln(ft_table, argv[1], argv[2], argv[3], argv[4], argv[5]);
    return 0;
}

// Function to insert a new parking history record into the database
void insertParkingHistory(sqlite3* db, const char* vehicleNumber, int spotNumber, time_t entryTime)
{
    sqlite3_stmt* stmt = NULL;
    char entryTimeStr[20];
    char exitTimeStr[20];
    int rc;

    struct tm timeInfo;
#ifdef _WIN32
    localtime_s(&timeInfo, &entryTime);
#else
    localtime_r(&entryTime, &timeInfo);
#endif
    strftime(entryTimeStr, sizeof(entryTimeStr), "%Y-%m-%d %H:%M:%S", &timeInfo);

    // Set a default exit time indicating the vehicle is still parked
    strcpy(exitTimeStr, "0000-00-00 00:00:00");

    const char* insertQuery = "INSERT INTO ParkingHistory (VehicleNumber, SpotNumber, EntryTime, ExitTime, ParkingFee) VALUES (?, ?, ?, ?, 0.0);";

    rc = sqlite3_prepare_v2(db, insertQuery, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare INSERT query: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, vehicleNumber, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, spotNumber);
    sqlite3_bind_text(stmt, 3, entryTimeStr, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, exitTimeStr, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
    }
    else
    {
        printf("Parking record inserted successfully.\n");
    }

    sqlite3_finalize(stmt);
}



// Function to update parking history record in the database
void updateParkingHistory(sqlite3* db, const char* vehicleNumber, time_t exitTime, double parkingFee)
{
    sqlite3_stmt* stmt = NULL;
    char exitTimeStr[20];
    int rc;

    struct tm timeInfo;
#ifdef _WIN32
    localtime_s(&timeInfo, &exitTime);
#else
    localtime_r(&exitTime, &timeInfo);
#endif
    strftime(exitTimeStr, sizeof(exitTimeStr), "%Y-%m-%d %H:%M:%S", &timeInfo);

    const char* updateQuery = "UPDATE ParkingHistory SET ExitTime = ?, ParkingFee = ? WHERE VehicleNumber = ? AND ExitTime = '0000-00-00 00:00:00';";

    rc = sqlite3_prepare_v2(db, updateQuery, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare UPDATE query: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, exitTimeStr, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, parkingFee);
    sqlite3_bind_text(stmt, 3, vehicleNumber, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
    }
    else
    {
        printf("Parking record updated successfully.\n");
    }

    sqlite3_finalize(stmt);
}

// Function to create the ParkingHistory table
void createTable(sqlite3* db)
{
    char* errMsg = 0;
    const char* sql = "CREATE TABLE IF NOT EXISTS ParkingHistory ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "VehicleNumber TEXT NOT NULL, "
        "SpotNumber INTEGER NOT NULL, "
        "EntryTime TEXT NOT NULL, "
        "ExitTime TEXT, "
        "ParkingFee REAL);";

    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    else
    {
        printf("Table created successfully.\n");
    }
}

