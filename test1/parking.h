#define _CRT_SECURE_NO_WARNINGS
#pragma once
#ifndef PARKING_H
#define PARKING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

#define MAX_PARKING_SPOTS 10 // Maximum parking spots available
#define FIXED_PARKING_RATE 30 // Fixed parking rate
#define OVERNIGHT_FEE 200 // Overnight parking fee
#define LOST_TICKET_FEE 300 // Lost ticket fee

// Structure to store information about vehicles
typedef struct {
    int id; // Vehicle ID
    char vehicleNumber[15]; // Vehicle Number (e.g., "ABC1234")
    time_t entryTime; // Entry time
    time_t exitTime; // Exit time
    int isParked; // 1 if parked, 0 if not
} Vehicle;

// Structure to store information about parking spots
typedef struct {
    int spotId; // Parking Spot ID
    int isOccupied; // 1 if occupied, 0 if free
    Vehicle vehicle; // Vehicle information for the parked vehicle
} ParkingSpot;

// Function prototypes
void showMenu();
void initializeParkingSpots(ParkingSpot parkingLot[], int totalSpots);
void parkVehicle(ParkingSpot parkingLot[], int totalSpots, sqlite3* db);
void removeVehicle(ParkingSpot parkingLot[], int totalSpots, sqlite3* db);
double calculateParkingFee(time_t entryTime, time_t exitTime);
void displayParkingRates();
void insertParkingHistory(sqlite3* db, const char* vehicleNumber, int spotNumber, time_t entryTime);
void updateParkingHistory(sqlite3* db, const char* vehicleNumber, time_t exitTime, double parkingFee);
void removeParkingHistory(sqlite3* db, const char* vehicleNumber);
void removeAllParkingHistory(sqlite3* db);
void createTable(sqlite3* db);

#endif // PARKING_H