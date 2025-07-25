typedef struct {
    const char *street;
    const char *town;
    int zip_code tag("ZipCode");
} Address;

typedef struct array() {
    int *items;
    size_t count;
    size_t capacity;
} Numbers;

typedef struct {
    const char *name;
    double age;
    Address address;
    Numbers lucky_numbers;
    double body_count tag("BodyCount");
    int height;
    bool married;
    bool dead;
} Person;

typedef struct array() {
    Person *items;
    size_t count;
    size_t capacity;
} People;
