#ifndef LABEL_MANAGER_HPP
#define LABEL_MANAGER_HPP

#include <stack>

using LabelID = int;

class LabelManager {
public:
    LabelManager() : labelCount(0) {}
    
    LabelID new_label() {
        return labelCount++;
    }

    // Only use with start_loop()
    LabelID test_loop() {
        int startLabel = new_label();
        loop_test_stack.push(startLabel);
        return startLabel;
    }
    
    // Only use with test_loop()
    LabelID start_loop() {
        int exitLabel = new_label();
        loop_exit_stack.push(exitLabel);
        return exitLabel;
    }

    // Requires a prior call to start_loop() AND test_loop()
    LabelID end_loop() {
        if(loop_exit_stack.size() == loop_test_stack.size()) {
            // loop incorrectly started, one of the start labels is missing
            return -1;
        }
        int exitLabel = loop_exit_stack.top();
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
    int labelCount;
    std::stack<int> loop_exit_stack;
    std::stack<int> loop_test_stack;
};



#endif // LABEL_MANAGER_HPP