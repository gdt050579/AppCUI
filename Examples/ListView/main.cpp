#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Console;

struct US_States {
    const char * Name;
    const char * Abbreviation;
    const char * Capital;
    const char * LargestCity;
    const char * Population;
    const char * Surface;
    const char * NrOfReps;
};
US_States us_states[] = {
    {"Alabama","AL","Montgomery","Birmingham","5,024,279","135,767","7"},
    {"Alaska","AK","Juneau","Anchorage","733,391","1,723,337","1"},
    {"Arizona","AZ","Phoenix","Phoenix","7,151,502","295,234","9"},
    {"Arkansas","AR","Little Rock","Little Rock","3,011,524","137,732","4"},
    {"California","CA","Sacramento","Los Angeles","39,538,223","423,967","53"},
    {"Colorado","CO","Denver","Denver","5,773,714","269,601","7"},
    {"Connecticut","CT","Hartford","Bridgeport","3,605,944","14,357","5"},
    {"Delaware","DE","Dover","Wilmington","989,948","6,446","1"},
    {"Florida","FL","Tallahassee","Jacksonville","21,538,187","170,312","27"},
    {"Georgia","GA","Atlanta","Atlanta","10,711,908","153,910","14"},
    {"Hawaii","HI","Honolulu","Honolulu","1,455,271","28,313","2"},
    {"Idaho","ID","Boise","Boise","1,839,106","216,443","2"},
    {"Illinois","IL","Springfield","Chicago","12,812,508","149,995","18"},
    {"Indiana","IN","Indianapolis","Indianapolis","6,785,528","94,326","9"},
    {"Iowa","IA","Des Moines","Des Moines","3,190,369","145,746","4"},
    {"Kansas","KS","Topeka","Wichita","2,937,880","213,100","4"},
    {"Kentucky[D]","KY","Frankfort","Louisville","4,505,836","104,656","6"},
    {"Louisiana","LA","Baton Rouge","New Orleans","4,657,757","135,659","6"},
    {"Maine","ME","Augusta","Portland","1,362,359","91,633","2"},
    {"Maryland","MD","Annapolis","Baltimore","6,177,224","32,131","8"},
    {"Massachusetts[D]","MA","Boston","Boston","7,029,917","27,336","9"},
    {"Michigan","MI","Lansing","Detroit","10,077,331","250,487","14"},
    {"Minnesota","MN","St. Paul","Minneapolis","5,706,494","225,163","8"},
    {"Mississippi","MS","Jackson","Jackson","2,961,279","125,438","4"},
    {"Missouri","MO","Jefferson City","Kansas City","6,154,913","180,540","8"},
    {"Montana","MT","Helena","Billings","1,084,225","380,831","1"},
    {"Nebraska","NE","Lincoln","Omaha","1,961,504","200,330","3"},
    {"Nevada","NV","Carson City","Las Vegas","3,104,614","286,380","4"},
    {"New Hampshire","NH","Concord","Manchester","1,377,529","24,214","2"},
    {"New Jersey","NJ","Trenton","Newark","9,288,994","22,591","12"},
    {"New Mexico","NM","Santa Fe","Albuquerque","2,117,522","314,917","3"},
    {"New York","NY","Albany","New York City","20,201,249","141,297","27"},
    {"North Carolina","NC","Raleigh","Charlotte","10,439,388","139,391","13"},
    {"North Dakota","ND","Bismarck","Fargo","779,094","183,108","1"},
    {"Ohio","OH","Columbus","Columbus","11,799,448","116,098","16"},
    {"Oklahoma","OK","Oklahoma City","Oklahoma City","3,959,353","181,037","5"},
    {"Oregon","OR","Salem","Portland","4,237,256","254,799","5"},
    {"Pennsylvania[D]","PA","Harrisburg","Philadelphia","13,002,700","119,280","18"},
    {"Rhode Island","RI","Providence","Providence","1,097,379","4,001","2"},
    {"South Carolina","SC","Columbia","Charleston","5,118,425","82,933","7"},
    {"South Dakota","SD","Pierre","Sioux Falls","886,667","199,729","1"},
    {"Tennessee","TN","Nashville","Nashville","6,910,840","109,153","9"},
    {"Texas","TX","Austin","Houston","29,145,505","695,662","36"},
    {"Utah","UT","Salt Lake City","Salt Lake City","3,271,616","219,882","4"},
    {"Vermont","VT","Montpelier","Burlington","643,077","24,906","1"},
    {"Virginia[D]","VA","Richmond","Virginia Beach","8,631,393","110,787","11"},
    {"Washington","WA","Olympia","Seattle","7,705,281","184,661","10"},
    {"West Virginia","WV","Charleston","Charleston","1,793,716","62,756","3"},
    {"Wisconsin","WI","Madison","Milwaukee","5,893,718","169,635","8"},
    {"Wyoming","WY","Cheyenne","Cheyenne","576,851","253,335","1"},
};


class MyWin : public AppCUI::Controls::Window
{
    ListView lv;

public:
    MyWin()
    {
        this->Create("List View Example", "a:c,w:70,h:20");
        lv.Create(this, "x:1,y:1,w:66,h:16", ListViewFlags::ALLOWSELECTION|ListViewFlags::HAS_CHECKBOX);
        lv.AddColumn("&State", TextAlignament::Left, 15);
        lv.AddColumn("&Abrv", TextAlignament::Center, 4);
        lv.AddColumn("&Capital", TextAlignament::Left, 10);
        lv.AddColumn("&Largest City", TextAlignament::Left, 15);
        lv.AddColumn("&Population", TextAlignament::Right, 12);
        lv.AddColumn("&Surface (km)", TextAlignament::Right, 12);
        lv.AddColumn("&Repr", TextAlignament::Center, 6);
        // add items
        lv.Reserve(100); // to populate the list faster
        for (unsigned int tr = 0; tr < sizeof(us_states) / sizeof(US_States); tr++)
        {
            lv.AddItem(us_states[tr].Name, us_states[tr].Abbreviation, us_states[tr].Capital, us_states[tr].LargestCity, us_states[tr].Population, us_states[tr].Surface, us_states[tr].NrOfReps);
        }
        // sort them after the name (first column)
        lv.Sort(0, true);
    }
    bool OnEvent(const void* sender, Event::Type eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        return false;
    }
};
int main()
{
    Application::Init(Application::Flags::HAS_COMMANDBAR);
    Application::AddWindow(new MyWin());
    Application::Run();
    return 0;
}
