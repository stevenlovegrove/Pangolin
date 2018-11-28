#include <iostream>
#include <pangolin/pangolin.h>

struct CustomType
{
    CustomType() = default;
    CustomType(int x, float y, const std::string& z)
        : x(x), y(y), z(z) {}

    int x;
    double y;
    std::string z;

    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( x, y, z );
    }
};

struct CustomType2
{
    CustomType2() = default;
    CustomType2(int x, float y, const std::string& z)
        : x(x), y(y), z(z){}

    int x;
    float y;
    std::string z;
    CustomType w;
};

template<typename T>
struct PangoVarDisplayTraits
{
    // Default - try to display as-is
};

template<>
struct PangoVarDisplayTraits<CustomType>
{
    template<typename Ar>
    void Display(Ar& ar, CustomType& t)
    {
//        pangolin::Var<int>Attach(t.x, )
    }
};

inline void MakeVoid() {}

class MyArchiver
{
public:
    MyArchiver()
        : depth(0), num(0)
    {
    }

    template<typename... Ts>
    void operator()(Ts&... ts)
    {
        ++depth;
        Recurse(ts...);
        --depth;
    }

    template<typename T>
    void AddLeaf(T& v)
    {
        std::string label = std::string("ui.") + std::string(depth,'_') + std::to_string(num);
        pangolin::Var<T>::Attach(label, v);
        typeids.push_back(label);
        ++num;
    }

private:

    template<typename T>
    void Recurse(T& v)
    {
        serialize(*this, v);
    }

    template<typename T, typename... Ts>
    void Recurse(T& v, Ts&... ts)
    {
        serialize(*this, v);
        Recurse(ts...);
    }

    size_t depth;
    size_t num;

public:
    std::vector<std::string> typeids;
};

void serialize(MyArchiver& ar, int& t) { ar.AddLeaf(t); }
void serialize(MyArchiver& ar, float& t) { ar.AddLeaf(t); }
void serialize(MyArchiver& ar, double& t) { ar.AddLeaf(t); }
void serialize(MyArchiver& ar, std::string& t) { ar.AddLeaf(t); }


void serialize(MyArchiver& ar, CustomType2& t)
{
    ar(t.x, t.y, t.z, t.w);
}

template<class Ar, class T>
auto serialize(Ar& ar, T& val)
 -> decltype(  std::declval<T&>().template serialize<Ar>(std::declval<Ar&>()) , MakeVoid() )
{
    val.serialize(ar);
}



class C {};

//int main2() {
//    std::cout << is_streamable2<std::stringstream, C>::value << std::endl;
//    std::cout << is_streamable2<std::stringstream, int>::value << std::endl;
//    return 0;
//}

int main(/*int argc, char* argv[]*/)
{  
    using namespace pangolin;

    MyArchiver ar;
    CustomType test1;
    CustomType2 test2;

    serialize(ar, test1);
    serialize(ar, test2);


    // Create OpenGL window in single line
    pangolin::CreateWindowAndBind("Main",640,480);
    glEnable(GL_DEPTH_TEST);

    // Define Camera Render Object (for view / scene browsing)
    pangolin::OpenGlRenderState s_cam(
                pangolin::ProjectionMatrix(640,480,420,420,320,240,0.1,1000),
                pangolin::ModelViewLookAt(-0,0.5,-3, 0,0,0, pangolin::AxisY)
                );

    const int UI_WIDTH = 300;

    // Add named OpenGL viewport to window and provide 3D Handler
    pangolin::View& d_cam = pangolin::CreateDisplay()
            .SetBounds(0.0, 1.0, pangolin::Attach::Pix(UI_WIDTH), 1.0, -640.0f/480.0f)
            .SetHandler(new pangolin::Handler3D(s_cam));

    // Add named Panel and bind to variables beginning 'ui'
    // A Panel is just a View with a default layout and input handling
    pangolin::CreatePanel("ui")
            .SetBounds(0.0, 1.0, 0.0, pangolin::Attach::Pix(UI_WIDTH));

//    pangolin::Var<CustomType> test("ui.test", CustomType(0,1,"test"));
//    pangolin::Var<int> test2("ui.test2", 1, 0, 5);
//    pangolin::Var<bool> test3("ui.test3", true, true);

    pangolin::Var<std::function<void(void)> >("ui.Reset", [](){
        std::cout << "Hello" << std::endl;
    });


    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(d_cam.IsShown()) {
            d_cam.Activate(s_cam);
            glColor3f(1.0,1.0,1.0);
            pangolin::glDrawColouredCube();
        }

        pangolin::FinishFrame();
    }

    return 0;
}
