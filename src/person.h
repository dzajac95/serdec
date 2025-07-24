typedef struct {
    const char *name;
    double age;
    const char *location;
    double body_count tag("BodyCount");
    int height;
    bool married;
    bool dead;
} Person;
