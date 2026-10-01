#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct Patient {
    std::string name;
    int waitingHours{};
    int arrivalMinutes{};
    bool lifeThreatening{};
    int remainingMinutes{25};
    int sequence{};
};

struct PatientPriority {
    bool operator()(const Patient& left, const Patient& right) const {
        if (left.lifeThreatening != right.lifeThreatening) {
            return left.lifeThreatening;
        }
        if (left.lifeThreatening) {
            return left.arrivalMinutes < right.arrivalMinutes;
        }
        if (left.waitingHours != right.waitingHours) {
            return left.waitingHours > right.waitingHours;
        }
        return left.sequence < right.sequence;
    }
};

struct StlPatientPriority {
    bool operator()(const Patient& left, const Patient& right) const {
        return PatientPriority{}(right, left);
    }
};

class AuthorPriorityQueue {
public:
    bool empty() const { return heap.empty(); }

    void push(const Patient& patient) {
        heap.push_back(patient);
        siftUp(heap.size() - 1);
    }

    Patient pop() {
        if (empty()) {
            throw std::out_of_range("pop() called on an empty priority queue");
        }
        Patient result = heap.front();
        heap.front() = heap.back();
        heap.pop_back();
        if (!empty()) {
            siftDown(0);
        }
        return result;
    }

private:
    std::vector<Patient> heap;
    PatientPriority higherPriority;

    void siftUp(std::size_t index) {
        while (index > 0) {
            std::size_t parent = (index - 1) / 2;
            if (!higherPriority(heap[index], heap[parent])) {
                break;
            }
            std::swap(heap[index], heap[parent]);
            index = parent;
        }
    }

    void siftDown(std::size_t index) {
        while (true) {
            std::size_t best = index;
            std::size_t left = index * 2 + 1;
            std::size_t right = left + 1;
            if (left < heap.size() && higherPriority(heap[left], heap[best])) {
                best = left;
            }
            if (right < heap.size() && higherPriority(heap[right], heap[best])) {
                best = right;
            }
            if (best == index) {
                return;
            }
            std::swap(heap[index], heap[best]);
            index = best;
        }
    }
};

struct Appointment {
    std::string patientName;
    int startMinutes{};
    int endMinutes{};
    bool interrupted{};
};

bool sameAppointment(const Appointment& left, const Appointment& right) {
    return left.patientName == right.patientName &&
           left.startMinutes == right.startMinutes &&
           left.endMinutes == right.endMinutes &&
           left.interrupted == right.interrupted;
}

struct SimulationResult {
    std::vector<Appointment> appointments;
};

Patient removeTop(AuthorPriorityQueue& queue) {
    return queue.pop();
}

template <typename Queue>
Patient removeTop(Queue& queue) {
    Patient patient = queue.top();
    queue.pop();
    return patient;
}

template <typename Queue>
SimulationResult simulate(std::vector<Patient> patients) {
    Queue queue;
    SimulationResult result;
    std::vector<bool> arrived(patients.size(), false);
    int currentTime = 12 * 60;
    Patient current;
    bool treating = false;

    auto addArrivals = [&]() {
        for (std::size_t index = 0; index < patients.size(); ++index) {
            if (!arrived[index] && patients[index].arrivalMinutes <= currentTime) {
                queue.push(patients[index]);
                arrived[index] = true;
            }
        }
    };

    auto nextArrival = [&]() {
        int next = 24 * 60;
        for (std::size_t index = 0; index < patients.size(); ++index) {
            if (!arrived[index] && patients[index].arrivalMinutes < next) {
                next = patients[index].arrivalMinutes;
            }
        }
        return next;
    };

    while (result.appointments.size() < patients.size() * 2) {
        addArrivals();
        if (!treating) {
            if (queue.empty()) {
                int next = nextArrival();
                if (next == 24 * 60) {
                    break;
                }
                currentTime = next;
                addArrivals();
            }
            if (!queue.empty()) {
                current = removeTop(queue);
                treating = true;
            }
        }
        if (!treating) {
            break;
        }

        int completion = currentTime + current.remainingMinutes;
        int next = nextArrival();
        if (next < completion) {
            current.remainingMinutes -= next - currentTime;
            result.appointments.push_back({current.name, currentTime, next, true});
            queue.push(current);
            currentTime = next;
            treating = false;
            addArrivals();
        } else {
            result.appointments.push_back({current.name, currentTime, completion, false});
            currentTime = completion;
            treating = false;
        }
    }
    return result;
}

int parseTime(const std::string& value) {
    int hour = 0;
    int minute = 0;
    char colon = '\0';
    std::istringstream input(value);
    if (!(input >> hour >> colon >> minute) || colon != ':') {
        throw std::runtime_error("Invalid time: " + value);
    }
    return hour * 60 + minute;
}

std::vector<Patient> defaultPatients() {
    return {
        {"Bob Bleeding", 4, 12 * 60, false, 25, 0},
        {"Frank Feelingbad", 5, 12 * 60, false, 25, 1},
        {"Cathy Coughing", 0, 12 * 60, false, 25, 2},
        {"Sam Sneezing", 0, 13 * 60 + 11, true, 25, 3},
        {"Paula Pain", 0, 14 * 60 + 16, true, 25, 4},
        {"Sid Sickly", 2, 12 * 60, false, 25, 5},
        {"Alice Ailment", 7, 12 * 60, false, 25, 6},
        {"Trent Ill", 1, 12 * 60, false, 25, 7},
        {"Tom Temperature", 6, 12 * 60, false, 25, 8}
    };
}

std::string clockTime(int totalMinutes) {
    int hour = (totalMinutes / 60) % 24;
    int minute = totalMinutes % 60;
    std::ostringstream output;
    output << hour << ':' << std::setw(2) << std::setfill('0') << minute;
    return output.str();
}

void printSchedule(const std::string& title, const SimulationResult& schedule) {
    std::cout << "\n" << title << "\n";
    std::cout << std::left << std::setw(22) << "Patient"
              << std::setw(10) << "Starts" << std::setw(10) << "Ends" << "Status\n";
    std::cout << std::string(58, '-') << '\n';
    for (const Appointment& appointment : schedule.appointments) {
        std::cout << std::left << std::setw(22) << appointment.patientName
                  << std::setw(10) << clockTime(appointment.startMinutes)
                  << std::setw(10) << clockTime(appointment.endMinutes)
                  << (appointment.interrupted ? "interrupted" : "complete") << '\n';
    }
}

int main() {
    try {
        std::vector<Patient> patients = defaultPatients();
        SimulationResult authorSchedule = simulate<AuthorPriorityQueue>(patients);
        SimulationResult stlSchedule = simulate<std::priority_queue<Patient, std::vector<Patient>, StlPatientPriority>>(patients);
        printSchedule("Priority Queue 1: author's heap", authorSchedule);
        printSchedule("Priority Queue 2: STL priority_queue", stlSchedule);

        bool schedulesMatch = authorSchedule.appointments.size() == stlSchedule.appointments.size();
        for (std::size_t index = 0; schedulesMatch && index < authorSchedule.appointments.size(); ++index) {
            schedulesMatch = sameAppointment(authorSchedule.appointments[index], stlSchedule.appointments[index]);
        }
        std::cout << "\nSchedules match: " << std::boolalpha << schedulesMatch << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}