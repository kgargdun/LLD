#include <iostream>
#include <string>
#include <vector>
using namespace std;

/* Memento */
class Memento {
public:
    Memento(string state) : state(state) {}
    string getState() const { return state; }

private:
    string state;
};

/* Originator */
class TextEditor {
public:
    void setText(const string& t) {
        text = t;
    }

    string getText() const {
        return text;
    }

    Memento createMemento() const {
        return Memento(text);
    }

    void restore(const Memento& m) {
        text = m.getState();
    }

private:
    string text;
};

/* Caretaker */
class History {
public:
    void save(const TextEditor& editor) {
        states.push_back(editor.createMemento());
    }

    void undo(TextEditor& editor) {
        if (states.empty()) return;
        editor.restore(states.back());
        states.pop_back();
    }

private:
    vector<Memento> states;
};

/* Usage */
int main() {
    TextEditor editor;
    History history;

    editor.setText("A");
    history.save(editor);

    editor.setText("AB");
    history.save(editor);

    editor.setText("ABC");
    cout << editor.getText() << "\n";   // ABC

    history.undo(editor);
    cout << editor.getText() << "\n";   // AB

    history.undo(editor);
    cout << editor.getText() << "\n";   // A
}
