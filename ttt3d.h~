#include <chrono>

/*
    Due June 1, 2015
    Each player gets a total of 3 minutes as measured by time_used()
    (x,y,z) in {0, 1, 2, 3}^3
*/

using namespace std::chrono;
using Clock = steady_clock;
using Duration = steady_clock::duration;
using Timepoint = steady_clock::time_point;

static double to_seconds(duration<double> d) {
    return d.count();
}

class TTT3D {
public:
    explicit TTT3D(const duration<double> TTA) : TTA(TTA) {
        t = Clock::now();
    }
 
    void init_clock() {
        tt = Clock::now() - t;
    }
 
    double time_used() const {
        if (!doing_move)
            return to_seconds(tt);
        else
            return to_seconds(tt + (Clock::now() - t));
    }
 
    void sqzzl(int mv[]) {
        doing_move = true;
        
        t = Clock::now();
        next_move(mv);
        tt += Clock::now() - t;
        
        doing_move = false;
    }
 
    // This is the method to be written for the assignment
    virtual void next_move(int mv[]) = 0;
 
private:
    bool doing_move = false;
    Timepoint t;
    Duration tt;
    const duration<double> TTA; // total time allowed for one player in seconds
};
