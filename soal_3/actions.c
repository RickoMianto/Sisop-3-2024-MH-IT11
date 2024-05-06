#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* Gap(float distance) {
    if (distance < 3.5)
        return "Gogogo";
    else if (distance >= 3.5 && distance <= 10)
        return "Push";
    else
        return "Stay out of trouble";
}

char* Fuel(int fuelPercentage) {
    if (fuelPercentage > 80)
        return "Push Push Push";
    else if (fuelPercentage >= 50 && fuelPercentage <= 80)
        return "You can go";
    else
        return "Conserve Fuel";
}

char* Tire(int tireUsage) {
    if (tireUsage > 80)
        return "Go Push Go Push";
    else if (tireUsage >= 50 && tireUsage <= 80)
        return "Good Tire Wear";
    else if (tireUsage >= 30 && tireUsage < 50)
        return "Conserve Your Tire";
    else
        return "Box Box Box";
}

char* TireChange(char* currentTireType) {
    if (strcmp(currentTireType, "Soft") == 0)
        return "Mediums Ready"; 
    else if (strcmp(currentTireType, "Medium") == 0)
        return "Box for Softs"; 
    else
        return "Invalid Tire Type"; 
}

int main() {
    float distance = 8.0;
    int fuelPercentage = 55;
    int tireUsage = 60;
    char currentTireType[] = "Medium";

    printf("Gap [%.1fs]: %s\n", distance, Gap(distance));
    printf("Fuel [%d%%]: %s\n", fuelPercentage, Fuel(fuelPercentage));
    printf("Tire [%d%%]: %s\n", tireUsage, Tire(tireUsage));
    printf("Tire Change [%s]: %s\n", currentTireType, TireChange(currentTireType));

    return 0;
}
