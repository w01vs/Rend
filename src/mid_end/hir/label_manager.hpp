#ifndef LABEL_MANAGER_HPP
#define LABEL_MANAGER_HPP

#include <stack>

struct LabelID {
    int value;
    explicit LabelID(int v) : value(v) {}
    int operator++(int other)
    {
        return ++value;
    }
};

class LabelManager {
public:
    LabelManager() : labelCount(0) {}
    
    LabelID new_label() {
        return LabelID(labelCount++);
    }

    // Only use with start_loop()
    LabelID test_loop() {
        LabelID startLabel = new_label();
        loop_test_stack.push(startLabel);
        return startLabel;
    }
    
    // Only use with test_loop()
    LabelID start_loop() {
        LabelID exitLabel = new_label();
        loop_exit_stack.push(exitLabel);
        return exitLabel;
    }

    // Requires a prior call to start_loop() AND test_loop()
    LabelID end_loop() {
        if(loop_exit_stack.size() == loop_test_stack.size()) {
            // loop incorrectly started, one of the start labels is missing
            return LabelID(-1);
        }
        LabelID exitLabel = loop_exit_stack.top();
        loop_exit_stack.pop();
        loop_test_stack.pop();
        return exitLabel;
    }

    LabelID get_loop_test_label() const {
        return loop_test_stack.top();
    }

    LabelID get_loop_exit_label() const {
        return loop_exit_stack.top();
    }
private:
    LabelID labelCount;
    std::stack<LabelID> loop_exit_stack;
    std::stack<LabelID> loop_test_stack;
};



#endif // LABEL_MANAGER_HPP