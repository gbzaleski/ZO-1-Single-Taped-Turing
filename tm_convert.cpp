#include <string>
#include "turing_machine.h"

using namespace std;

// Translation from two-taped Turing Machine to single-taped Turing Machine.
// By Grzegorz B. Zaleski (418494), University of Warsaw
// Complexity O(n) (for each step of original machine)
// Alphabet size O(m^2) (for m size of original machine)

// Transition signature
// typedef std::map<
//         std::pair<std::string, std::vector<std::string>>, 
//         std::tuple<std::string, std::vector<std::string>, std::string>
//     > transitions_t;

// Two-tape: (state, [let1, let2] -> (new_state, [let1, let2], "move1 move2"))
// One-tape: (state, [let] -> (new_state, [let], "move"))

// Move phases for simulting single two-taped moved
#define PHASE0_START "Phase0-Start"
#define PHASE0_INPUT "Phase0-Input"
#define PHASE0_BACK  "Phase0-Back"
#define PHASE0_SETUP_MARKS "Phase0-Setup-Marks"

#define PHASE1_FIND_SECOND "Phase1-Find-Second" // (Keeping in extended state letter from second head)
#define PHASE1_SET_SECOND_MARK "Phase1-Set-Second-Mark" 
#define PHASE1_BACK "Phase1-Back"

#define PHASE2_FIND_FIRST "Phase2-Find-First" 
#define PHASE2_SET_FIRST_MARK "Phase2-Set-First-Mark" // Set new letter (add head marker), save what letter is there
#define PHASE2_BACK "Phase2-Back"

// New State = (Phase x Original_State x Alphabet x Direction)

// F' : (Current_phase x Old_state x Letter to/from head1 x Move for head1), [Letter at head2]
//      ---> (New_phase x Next_old_state x Letter to/from head1 x move for head1), [New letter at head2], Move

// Adds brackets if necessary
bool is_wrapped(const string &inp)
{
    int cnt = 0;
    for (size_t i = 0; i < inp.size(); ++i)
    {
        if (inp[i] == '(')
            cnt++;
        else if (inp[i] == ')')
            cnt--;
        
        if (cnt < 1 && i != inp.size() - 1)
            return false;
    }
    return true;
}
inline string wrap (const string &inp)
{
    return (inp.size() < 2 || is_wrapped(inp)) ? inp : "(" + inp + ")";
}

// Append transitiion for one-taped machine.
inline void append_transitions(transitions_t &transitions, 
    const string &from_state, const string &from_letter_at_head, 
    string new_state, const string &new_letter_at_head, const string &head_direction)
{
    const string SIGN = "(-)";
    // Accept state concludes programme.
    if (new_state != ACCEPTING_STATE && from_state.find(SIGN + ACCEPTING_STATE + SIGN) != std::string::npos)
        return;

    pair<std::string, std::vector<std::string>> key = make_pair(wrap(from_state), std::vector<std::string>{wrap(from_letter_at_head)});
    tuple<std::string, std::vector<std::string>, std::string> value = make_tuple(wrap(new_state), std::vector<std::string>{wrap(new_letter_at_head)}, head_direction);

    transitions[key] = value;
}

// Retrieves letter from the new state
string get_letter(const string &state_value)
{
    int i = 0;
    string letter = "";
    // Phase
    while (state_value[i] != '(' || state_value[i+1] != '-' || state_value[i+2] != ')')
    {
        i++;
    }
    i++;
    i++;
    i++;

    // State
    while (state_value[i] != '(' || state_value[i+1] != '-' || state_value[i+2] != ')')
    {
        i++;
    }
    i++;
    i++;
    i++;

    while (state_value[i] != '(' || state_value[i+1] != '-' || state_value[i+2] != ')')
    {
        letter += state_value[i++];
    }

    return letter;
}

// Retrieves the original state from the new state
string get_state(const string &state_value)
{
    int i = 0;
    string state = "";
    // Phase
    while (state_value[i] != '(' || state_value[i+1] != '-' || state_value[i+2] != ')')
    {
        i++;
    }
    i++;
    i++;
    i++;

    // State
    while (state_value[i] != '(' || state_value[i+1] != '-' || state_value[i+2] != ')')
    {
        state += state_value[i++];
    }

    return state;
}

// For saving direction in the state
inline char direction_from_chr(char c)
{
    if (c == 'L')
        return '<';
    if (c == 'R')
        return '>';
    return c;
}
inline char direction_to_chr(char c)
{
    if (c == '<')
        return 'L';
    if (c == '>')
        return 'R';
    return c;
}

// Converts two-taped Turing Machine to single-taped Turing Machine
TuringMachine tm_convert(const TuringMachine original_tm)
{
    if (original_tm.num_tapes != 2)
    {
        cout << "Provided machine is not two-taped!\n";
        exit(1);
    }

    const string SIGN = "(-)";
    const string GUARD = SIGN + SIGN;
    const string HEAD = "v";

    auto original_tm_work_alphabet_with_blank = original_tm.working_alphabet();
    original_tm_work_alphabet_with_blank.push_back(BLANK);
    transitions_t ottm_transitions;

    // Preparing input
    for (auto orig_letter : original_tm.input_alphabet)
    { 
        auto phase_start =  (PHASE0_START + SIGN + INITIAL_STATE + SIGN + BLANK + SIGN + BLANK);
        append_transitions(ottm_transitions, INITIAL_STATE, orig_letter, 
           phase_start, orig_letter, string{HEAD_STAY});

        auto phase_input = (PHASE0_INPUT + SIGN + INITIAL_STATE + SIGN + orig_letter + SIGN + BLANK);
        append_transitions(ottm_transitions, phase_start, orig_letter, 
            phase_input, GUARD, string{HEAD_RIGHT});

        for (auto letter_to_see : original_tm.input_alphabet)
        {
            auto phase_next_input = (PHASE0_INPUT + SIGN + INITIAL_STATE + SIGN + letter_to_see + SIGN + BLANK);
            append_transitions(ottm_transitions, phase_input, letter_to_see,
                phase_next_input, orig_letter + SIGN + BLANK, string{HEAD_RIGHT});
        }

        auto phase_go_back = (PHASE0_INPUT + SIGN + INITIAL_STATE + SIGN + BLANK + SIGN + BLANK);
        append_transitions(ottm_transitions, phase_input, BLANK,
            phase_go_back, orig_letter + SIGN + BLANK, string{HEAD_LEFT});

        append_transitions(ottm_transitions, phase_go_back, orig_letter + SIGN + BLANK, 
            phase_go_back, orig_letter + SIGN + BLANK, string{HEAD_LEFT});

        auto phase_setup_marks = (PHASE0_SETUP_MARKS + SIGN + INITIAL_STATE + SIGN + BLANK + SIGN + BLANK);
        append_transitions(ottm_transitions, phase_go_back, GUARD,
            phase_setup_marks, GUARD, string{HEAD_RIGHT});

        auto phase_start_work = (PHASE1_FIND_SECOND + SIGN + INITIAL_STATE + SIGN + orig_letter + SIGN + BLANK);
        append_transitions(ottm_transitions, phase_setup_marks, orig_letter + SIGN + BLANK,
           phase_start_work, HEAD + orig_letter + SIGN + HEAD + BLANK, string{HEAD_STAY});
    }

    // Phase 1 - Transition translation
    // Two-tape: (state, [let1, let2] -> (new_state, [let1, let2], "move1 move2"))
    for (auto &[k, v] : original_tm.transitions)
    {
        // Phase $ State Before $ Letter at head 1 $ Direction (null) 
        auto state_before = PHASE1_FIND_SECOND + SIGN + k.first + SIGN + k.second[0] + SIGN + BLANK;
        // Phase $ State Now $ New letter for head 1 $ Direction for 1
        auto state_after = PHASE1_SET_SECOND_MARK + SIGN + get<0>(v) + SIGN + get<1>(v)[0] + SIGN + direction_to_chr(get<2>(v)[0]);

        for (auto letter_on_first : original_tm_work_alphabet_with_blank)
        {
            append_transitions(ottm_transitions, state_before, letter_on_first + SIGN + HEAD + k.second[1],
                state_after, letter_on_first + SIGN + get<1>(v)[1], string{get<2>(v)[1]});
                
            append_transitions(ottm_transitions, state_before, HEAD + letter_on_first + SIGN + HEAD + k.second[1],
                state_after, HEAD + letter_on_first + SIGN + get<1>(v)[1], string{get<2>(v)[1]});
        }
    }

    // Phase 1 - Setting mark of second head
    for (auto letter_on_first : original_tm_work_alphabet_with_blank)
    {
        for (auto letter_on_second : original_tm_work_alphabet_with_blank)
        {
            for (auto &[k, v] : ottm_transitions)
            {
                string current_state = get<0>(v);
                if (current_state.find(PHASE1_SET_SECOND_MARK) != std::string::npos)
                {
                    auto state_after = "(" + string{PHASE1_BACK} + current_state.substr(23);

                    append_transitions(ottm_transitions, current_state, letter_on_first + SIGN + letter_on_second,
                        state_after, letter_on_first + SIGN + HEAD + letter_on_second, string{HEAD_LEFT});

                    append_transitions(ottm_transitions, current_state, HEAD + letter_on_first + SIGN + letter_on_second,
                        state_after, HEAD + letter_on_first + SIGN + HEAD + letter_on_second, string{HEAD_LEFT});
                }
            }
        }
    }

    // Phase 1 - Backing
    for (auto &[k, v] : ottm_transitions)
    {
        string current_state = get<0>(v);
        

        for (auto letter_on_first : original_tm_work_alphabet_with_blank)
        {
            for (auto letter_on_second : original_tm_work_alphabet_with_blank)
            {
                
                if (current_state.find(PHASE1_BACK) != std::string::npos)
                {
                    // State does not change until reaching guard.
                    auto state_after = current_state;
                    auto cell_at_head = letter_on_first + SIGN + letter_on_second;

                    append_transitions(ottm_transitions, current_state, cell_at_head,
                        state_after, cell_at_head, string{HEAD_LEFT});

                    append_transitions(ottm_transitions, current_state, HEAD + cell_at_head,
                        state_after, HEAD + cell_at_head, string{HEAD_LEFT});
                }
            }
        }

        if (current_state.find(PHASE1_BACK) != std::string::npos) 
        {
            auto state_find_head1 = "(" + string{PHASE2_FIND_FIRST} + current_state.substr(12);
            append_transitions(ottm_transitions, current_state, GUARD, 
                 state_find_head1, GUARD, string{HEAD_RIGHT});
        }
    }

    // Phase 2 - Find first head 
    for (auto &[k, v] : ottm_transitions)
    {
        string current_state = get<0>(v);
        

        for (auto letter_on_first : original_tm_work_alphabet_with_blank)
        {
            for (auto letter_on_second : original_tm_work_alphabet_with_blank)
            {
                
                if (current_state.find(PHASE2_FIND_FIRST) != std::string::npos)
                {
                    
                    auto state_after = current_state;
                    auto cell_at_head = letter_on_first + SIGN + letter_on_second;
                    auto cell_at_head_second_head = letter_on_first + SIGN + HEAD + letter_on_second;

                    // Does not see head -> move next
                    append_transitions(ottm_transitions, current_state, cell_at_head,
                        state_after, cell_at_head, string{HEAD_RIGHT});
                    append_transitions(ottm_transitions, current_state, cell_at_head_second_head,
                        state_after, cell_at_head_second_head, string{HEAD_RIGHT});

                    // Sees head (at first) -> next phase (overwrite letter)
                    state_after = wrap(string{PHASE2_SET_FIRST_MARK} + SIGN + get_state(current_state) + SIGN + BLANK + SIGN + BLANK);
                    char new_direction = direction_from_chr(current_state[current_state.size() - 2]);
                    string new_letter = get_letter(current_state) + SIGN + letter_on_second;
                    append_transitions(ottm_transitions, current_state, HEAD + cell_at_head,
                        state_after, new_letter, string{new_direction});

                    string new_letter_head_at_second = get_letter(current_state) + SIGN + HEAD + letter_on_second;
                    append_transitions(ottm_transitions, current_state, HEAD + cell_at_head_second_head,
                        state_after, new_letter_head_at_second, string{new_direction});
                }
            }
        }
    }

    // Phase 2 - Mark first head (and remember its value)
    for (auto &[k, v] : ottm_transitions)
    {
        string current_state = get<0>(v);
        
        for (auto letter_on_first : original_tm_work_alphabet_with_blank)
        {
            for (auto letter_on_second : original_tm_work_alphabet_with_blank)
            {
                if (current_state.find(PHASE2_SET_FIRST_MARK) != std::string::npos)
                {
                    auto cell_at_head = letter_on_first + SIGN + letter_on_second;
                    auto cell_at_head_second_head = letter_on_first + SIGN + HEAD + letter_on_second;
                    auto state_after = wrap(string{PHASE2_BACK} + SIGN + get_state(current_state) + SIGN + letter_on_first + SIGN + BLANK);

                    append_transitions(ottm_transitions, current_state, cell_at_head,
                        state_after, HEAD + cell_at_head, string{HEAD_LEFT});

                    append_transitions(ottm_transitions, current_state, cell_at_head_second_head,
                        state_after, HEAD + cell_at_head_second_head, string{HEAD_LEFT});
                }
            }
        }
    }


    // Phase 2 - Backing 
    for (auto &[k, v] : ottm_transitions)
    {
        string current_state = get<0>(v);
        

        for (auto letter_on_first : original_tm_work_alphabet_with_blank)
        {
            for (auto letter_on_second : original_tm_work_alphabet_with_blank)
            {
                
                if (current_state.find(PHASE2_BACK) != std::string::npos)
                {
                    // State does not change until reaching guard.
                    auto state_after = current_state;
                    auto cell_at_head = letter_on_first + SIGN + letter_on_second;
                    auto cell_at_head_second_head = letter_on_first + SIGN + HEAD + letter_on_second;

                    append_transitions(ottm_transitions, current_state, cell_at_head,
                        state_after, cell_at_head, string{HEAD_LEFT});

                    append_transitions(ottm_transitions, current_state, cell_at_head_second_head,
                        state_after, cell_at_head_second_head, string{HEAD_LEFT});
                }
            }
        }

        if (current_state.find(PHASE2_BACK) != std::string::npos) 
        {
            auto state_find_head2 = "(" + string{PHASE1_FIND_SECOND} + current_state.substr(12);
            append_transitions(ottm_transitions, current_state, GUARD, 
                 state_find_head2, GUARD, string{HEAD_RIGHT});
        }
    }


    // Phase 1 - Find second head (keeping in memory first's value)
    for (auto &[k, v] : ottm_transitions)
    {
        string current_state = get<0>(v);
        
        for (auto letter_on_first : original_tm_work_alphabet_with_blank)
        {
            for (auto letter_on_second : original_tm_work_alphabet_with_blank)
            {
                
                if (current_state.find(PHASE1_FIND_SECOND) != std::string::npos)
                {
                    
                    auto state_after = current_state;
                    auto cell_at_head = letter_on_first + SIGN + letter_on_second;
                    auto cell_at_head_first_head = HEAD + letter_on_first + SIGN + letter_on_second;

                    // Does not see head at second head -> move next
                    append_transitions(ottm_transitions, current_state, cell_at_head,
                        state_after, cell_at_head, string{HEAD_RIGHT});
                    append_transitions(ottm_transitions, current_state, cell_at_head_first_head,
                        state_after, cell_at_head_first_head, string{HEAD_RIGHT});

                    // Sees head (at first) -> next phase (overwrite letter)
                    // Already introduced in first step = loop completed.
                }
            }
        }
    }


    // Extending Blanks
    for (auto &[k, v] : ottm_transitions)
    {
        if (k.first.substr(1, 6) == "Phase1" || k.first.substr(1, 6) == "Phase2")
        {
            append_transitions(ottm_transitions, k.first, BLANK, 
                    k.first, BLANK + SIGN + BLANK, string{HEAD_STAY});
        }
    }
    
    // Empty input corner case
    const vector<string> EMPTY_CELLS = {BLANK, BLANK};
    if (original_tm.transitions.find(make_pair(INITIAL_STATE, EMPTY_CELLS)) != original_tm.transitions.end())
    {
        auto transitions_corner_state = INITIAL_STATE + SIGN + INITIAL_STATE + SIGN + BLANK + SIGN + BLANK;
        append_transitions(ottm_transitions, INITIAL_STATE, BLANK,
            transitions_corner_state, GUARD, string{HEAD_RIGHT});

        auto new_state = PHASE1_FIND_SECOND + SIGN + INITIAL_STATE + SIGN + BLANK + SIGN + BLANK;
            append_transitions(ottm_transitions, transitions_corner_state, BLANK,
                new_state, HEAD + BLANK + SIGN + HEAD + BLANK, string{HEAD_STAY});
    }

    // Accept translation
    for (auto &[k, v] : ottm_transitions)
    {
        auto current_state = get<0>(v);
        // Reaching accepting state concludes programme - if head does not fall from the tape.
        if (current_state != ACCEPTING_STATE && current_state.find(SIGN + ACCEPTING_STATE + SIGN) != std::string::npos)
        {
            append_transitions(ottm_transitions, current_state, BLANK, ACCEPTING_STATE, BLANK, string{HEAD_STAY});
            
            for (auto letter_on_first : original_tm_work_alphabet_with_blank)
            {
                for (auto letter_on_second : original_tm_work_alphabet_with_blank)
                {
                    auto cell_at_head = letter_on_first + SIGN + letter_on_second;
                    auto cell_at_head_second_head = letter_on_first + SIGN + HEAD + letter_on_second;

                    // Move to the accept state
                    append_transitions(ottm_transitions, current_state, cell_at_head,
                        ACCEPTING_STATE, cell_at_head, string{HEAD_STAY});
                    append_transitions(ottm_transitions, current_state, cell_at_head_second_head,
                        ACCEPTING_STATE, cell_at_head_second_head, string{HEAD_STAY});
                    append_transitions(ottm_transitions, current_state, HEAD + cell_at_head,
                        ACCEPTING_STATE, HEAD + cell_at_head, string{HEAD_STAY});
                    append_transitions(ottm_transitions, current_state, HEAD + cell_at_head_second_head,
                        ACCEPTING_STATE, HEAD + cell_at_head_second_head, string{HEAD_STAY});
                }
            }
        }
    }

    return TuringMachine(1, original_tm.input_alphabet, ottm_transitions);
}
