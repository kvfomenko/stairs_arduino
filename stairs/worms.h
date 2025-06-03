
struct Worm {
  int step;
  int point;
  int animation_frame;
};

class WormsManager {
  private:
    static const int MAX_WORMS = 30; // Maximum number of worms in the array
    int num_worms;                  // Current number of worms in the array

  public:
    Worm worms[MAX_WORMS];          // Array to store Worms

    // Constructor to initialize the array
    WormsManager() {
      num_worms = 0; // Initialize the number of worms to 0
    }

    // Add a worm to the end of the array
    bool add(int step, int point, int animation_frame) {
      if (num_worms < MAX_WORMS) {
        worms[num_worms].step = step;
        worms[num_worms].point = point;
        worms[num_worms].animation_frame = animation_frame;
        num_worms++;
        return true; // Successfully added
      }
      return false; // Failed, array is full
    }

    // Remove a worm at a specific index (specified by index)
    bool del(int index) {
      if (index >= 0 && index < num_worms) {
        // Shift all elements after the specified index one position to the left
        for (int i = index; i < num_worms - 1; i++) {
          worms[i] = worms[i + 1];
        }
        num_worms--; // Decrease the count of worms
        return true; // Successfully removed
      }
      return false; // Failed, index out of range
    }

    // Clear all worms from the array
    void clear() {
      num_worms = 0; // Simply reset the worm count to 0
    }

    // Get the number of worms currently in the array
    int size() {
      return num_worms;
    }
};

WormsManager wormManager;
