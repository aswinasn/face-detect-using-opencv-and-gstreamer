
//#include "elements.cpp"
#include "face_detect_by_image.cpp"
//#include "face_detect_by_video.cpp"

// void face_detect_by_video();
// void face_detect_by_image();

//// options -----------------------------------------------

void options(){

    cout << "\n choose '1' to detect face from video";
    cout << "\n choose '2' to detect face from image ";
    cout << "\n choose '3' to exit the process ";

}

///// selecting options -----------------------------------

void select(){
    int option;
    options();
    cout << "\n enter the option : ";
    cin >> option;
    do{
        switch(option){
            case 1: face_detect_by_video();
                    options();
                    cout << "\n enter the option : ";
                    cin >> option;
                    break;

            case 2: face_detect_by_image();
                    options();
                    cout << "\n enter the option : ";
                    cin >> option;
                    break;

            case 3: exit(0);

            default : cout << "\n enter invalid option choose another one : ";
                      options();
                      cout << "\n enter valid option : ";
                      cin >> option;
                      break;
        }
    }while(option!=4);
}

int main(){

    select();

    return 0;
}