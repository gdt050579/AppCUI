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

#define SHOW_DEFAULT_EXAMPLE    1000
#define MY_GROUP                123

class MyDialog : public AppCUI::Controls::Window
{
public:
    bool OnEvent(const void* sender, Event::Type eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            this->Exit((int)Dialogs::DialogResult::RESULT_OK);
            return true;
        }
        return false;
    }
};

class MyListViewExample : public MyDialog
{
    ListView lv;
public:
    MyListViewExample(ListViewFlags flags)
    {
        this->Create("List View Example", "a:c,w:70,h:20");
        lv.Create(this, "x:1,y:1,w:66,h:16", flags);
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
};

class SimpleListExample : public MyDialog
{    
    ListView lv;
public:
    SimpleListExample(bool hasCheckboxes)
    {
        this->Create("Simple List Example", "a:c,w:30,h:14");
        lv.Create(this, "x:1,y:1,w:26,h:10", hasCheckboxes ? (ListViewFlags::HAS_CHECKBOXES|ListViewFlags::HIDE_COLUMNS) : ListViewFlags::HIDE_COLUMNS);
        lv.AddColumn("", TextAlignament::Left, 30);
        lv.AddItem("Apple"); lv.AddItem("Pinaple"); lv.AddItem("Pears"); lv.AddItem("Lemons"); lv.AddItem("Oranges");
    }
};
class ColumnsExample : public MyDialog
{
    ListView lv;
    Label inf;
public:
    ColumnsExample()
    {
        this->Create("Columns/Headers Example", "a:c,w:70,h:18");
        lv.Create(this, "x:1,y:7,w:66,h:8", ListViewFlags::SORTABLE);
        // columns
        lv.AddColumn("&Name", TextAlignament::Left, 30);
        lv.AddColumn("Cl&ass", TextAlignament::Left, 20);
        lv.AddColumn("&Grade", TextAlignament::Right, 7);
        // items
        lv.AddItem("Mike", "Mathematics", "9");
        lv.AddItem("Laura", "Mathematics", "7");
        lv.AddItem("John", "Phishics", "8");
        lv.AddItem("Ana", "Chemestry", "9");
        lv.AddItem("Willian", "Literature", "6");

        inf.Create(this, "Use Ctrl+Left/Ritgh to select a column and then Left or Right to change its size. You can also hover the mouse over a column separator to resize it.\nClick on each column to sort items based on that column.\nYou can also press Ctrl+N (to sort after the first column), Ctrl+A and Ctrl+G to sort using the next two ones", "x:1,y:1,w:66,h:6");
    }
};

class MyWin : public AppCUI::Controls::Window
{
    Panel p;
    CheckBox cbHideColumns, cbCheckBoxes, cbHideColumnSeparators, cbSort,cbItemSeparators, cbAllowSelection, cbSimpleListCheckboxes;
    RadioBox rbCustomizedListView, rbSimpleList, rbSortAndColumnsFeatures;
    Button btnShow;
public:
    MyWin()
    {
        this->Create("ListView example config", "x:0,y:0,w:60,h:20");
        rbCustomizedListView.Create(this, "USA states (a generic list with different features)", "x:1,y:1,w:56", MY_GROUP);
        p.Create(this,"x:4,y:2,w:56,h:6");
        cbHideColumns.Create(&p, "&Hide columns (item headers)", "x:1,y:0,w:50");
        cbCheckBoxes.Create(&p, "&Checkboxes (each item is checkable)", "x:1,y:1,w:50");
        cbHideColumnSeparators.Create(&p, "Hide column separators (&vertical lines)", "x:1,y:2,w:50");
        cbSort.Create(&p, "&Sortable (columns can be used to sort)", "x:1,y:3,w:50");
        cbItemSeparators.Create(&p, "Add a line after each item (item separators)", "x:1,y:4,w:50");
        cbAllowSelection.Create(&p, "Enable &multiple selections mode", "x:1,y:5,w:50");

        rbSimpleList.Create(this, "A very simple list with items", "x:1,y:8,w:56", MY_GROUP);
        cbSimpleListCheckboxes.Create(this, "Has checkboxes", "x:5,y:9,w:30");

        rbSortAndColumnsFeatures.Create(this, "Columns example (sort & resize)", "x:1,y:10,w:56", MY_GROUP);

        rbCustomizedListView.SetChecked(true);
        btnShow.Create(this, "Show example", "l:14,b:0,w:21", SHOW_DEFAULT_EXAMPLE);

        UpdateFeaturesEnableStatus();
    }
    void UpdateFeaturesEnableStatus()
    {
        cbHideColumns.SetEnabled(rbCustomizedListView.IsChecked());
        cbCheckBoxes.SetEnabled(rbCustomizedListView.IsChecked());
        cbHideColumnSeparators.SetEnabled(rbCustomizedListView.IsChecked());
        cbSort.SetEnabled(rbCustomizedListView.IsChecked());
        cbItemSeparators.SetEnabled(rbCustomizedListView.IsChecked());
        cbAllowSelection.SetEnabled(rbCustomizedListView.IsChecked());
        cbSimpleListCheckboxes.SetEnabled(rbSimpleList.IsChecked());
    }
    void ShowListView()
    {
        ListViewFlags flags = ListViewFlags::NONE;

        if (cbHideColumns.IsChecked())
            flags = flags | ListViewFlags::HIDE_COLUMNS;
        if (cbCheckBoxes.IsChecked())
            flags = flags | ListViewFlags::HAS_CHECKBOXES;
        if (cbHideColumnSeparators.IsChecked())
            flags = flags | ListViewFlags::HIDE_COLUMNS_SEPARATORS;
        if (cbSort.IsChecked())
            flags = flags | ListViewFlags::SORTABLE;
        if (cbItemSeparators.IsChecked())
            flags = flags | ListViewFlags::ITEM_SEPARATORS;
        if (cbAllowSelection.IsChecked())
            flags = flags | ListViewFlags::MULTIPLE_SELECTION_MODE;

        if (rbCustomizedListView.IsChecked())
        {
            MyListViewExample win(flags);
            win.Show();
        }
        if (rbSimpleList.IsChecked())
        {
            SimpleListExample win(cbSimpleListCheckboxes.IsChecked());
            win.Show();
        }
        if (rbSortAndColumnsFeatures.IsChecked())
        {
            ColumnsExample win;
            win.Show();
        }
    }
    bool OnEvent(const void* sender, Event::Type eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::EVENT_CHECKED_STATUS_CHANGED)
        {
            UpdateFeaturesEnableStatus();
            return true;
        }
        if ((eventType == Event::EVENT_BUTTON_CLICKED) && (controlID == SHOW_DEFAULT_EXAMPLE))
        {
            ShowListView();
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
