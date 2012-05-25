#include "input_record_repeat.h"

using namespace std;

namespace pangolin
{

InputRecordRepeat::InputRecordRepeat(const std::string& var_record_prefix)
    : record(false), play(false), index(-1)
{
    RegisterGuiVarChangedCallback(&InputRecordRepeat::GuiVarChanged,(void*)this,var_record_prefix);
}

InputRecordRepeat::~InputRecordRepeat()
{

}

void InputRecordRepeat::SetIndex(int id)
{
//    if( id < index )
//        Clear();

    index = id;

    while( !play_queue.empty() && play_queue.front().index < index )
    {
        // 'Play' Frameinput
        FrameInput in = play_queue.front();
        play_queue.pop_front();
        Var<std::string> var(in.var);
        var = in.val;
    }
}

void InputRecordRepeat::Record()
{
    ClearBuffer();
    play = false;
    record = true;
}

void InputRecordRepeat::Stop()
{
    record = false;
    play = false;
}

void InputRecordRepeat::ClearBuffer()
{
    index = -1;
    record_queue.clear();
    play_queue.clear();
}

ostream& operator<<(ostream& os, const FrameInput& fi )
{
    os << fi.index << endl << fi.var << endl << fi.val << endl;
    return os;
}

istream& operator>>(istream& is, FrameInput& fi)
{
    is >> fi.index;
    is.ignore(std::numeric_limits<streamsize>::max(),'\n');
    getline(is,fi.var);
    getline(is,fi.val);
    return is;
}

void InputRecordRepeat::SaveBuffer(const std::string& filename)
{
    ofstream f(filename.c_str());

    for( std::list<FrameInput>::const_iterator i = record_queue.begin(); i!=record_queue.end(); ++i )
    {
        f << *i;
    }
}

void InputRecordRepeat::LoadBuffer(const std::string& filename)
{
    record_queue.clear();

    ifstream f(filename.c_str());
    while(f.good())
    {
        FrameInput fi;
        f >> fi;
        if( f.good() )
            record_queue.push_back(fi);
    }
}

void InputRecordRepeat::PlayBuffer()
{
    play_queue = record_queue;

    record = false;
    play = true;
}

void InputRecordRepeat::PlayBuffer(int start, int end)
{
    std::list<FrameInput>::iterator s = record_queue.begin();
    std::list<FrameInput>::iterator e = record_queue.begin();

    for(int i=0; i<start; i++) s++;
    for(int i=0; i<end; i++) e++;

    play_queue.clear();
    play_queue.insert(play_queue.begin(),s,e);

    record = false;
    play = true;
}

int InputRecordRepeat::Size()
{
    return record_queue.size();
}

void InputRecordRepeat::UpdateVariable(const std::string& name )
{
    Var<std::string> var(name);

    if( record )
    {
        FrameInput input;
        input.index = index;
        input.var = name;
        input.val = var.a->Get();
        record_queue.push_back(input);
    }
}

void InputRecordRepeat::GuiVarChanged(void* data, const std::string& name, _Var& _var)
{
    InputRecordRepeat* thisptr = (InputRecordRepeat*)data;

    if( thisptr->record )
    {
        Var<std::string> var(_var);

        FrameInput input;
        input.index = thisptr->index;
        input.var = name;
        input.val = var.a->Get();

        thisptr->record_queue.push_back(input);
    }
}

}
