#define CATCH_CONFIG_MAIN
#if __has_include(<catch2/catch.hpp>)
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <pangolin/var/var.h>
#include <pangolin/var/varextra.h>

using namespace pangolin;

struct CustomType{
    float a; int b;
};
std::ostream& operator<<(std::ostream& os, const CustomType& x)
{
    return os << x.a << "," << x.b;
}
std::istream& operator>>(std::istream& is, CustomType& x)
{
    is >> x.a;
    if( is.get() != ',') throw BadInputException();
    is >> x.b;
    return is;
}

struct CustomTypeNoStream{
    float a; int b;
};


SCENARIO( "Native Var Types" ) {
    // Pangolin Var's have enduring lifetime and we must reset state
    VarState::I().Clear();

    GIVEN( "A Var with some type" ) {
        Var<double> x1("some_double", 3.14);

        REQUIRE( x1 == 3.14 );
        REQUIRE( x1.Get() == 3.14 );
        REQUIRE( x1.Meta().full_name == "some_double" );
        REQUIRE( x1.Meta().friendly == "some_double" );
        REQUIRE( x1.Meta().generic == false );

        WHEN( "the Var is set" ) {
            x1 = 6.7;

            THEN( "the Var and underlying variable change" ) {
                REQUIRE( x1 == 6.7 );
                REQUIRE( x1.Get() == 6.7 );
            }
        }

        WHEN( "a new Var with the same name is created" ) {
            Var<double> alias_x1("some_double", 9.2);

            THEN( "the Var references the original unchanged value" ) {
                REQUIRE( alias_x1 == x1);
                REQUIRE( alias_x1 == 3.14);
                REQUIRE( &(alias_x1.Get()) == &(x1.Get()) );
            }

            WHEN( "the original Var is changed") {
                x1 = 10.2;

                THEN( "The new Var also changes") {
                    REQUIRE(alias_x1 == 10.2);
                }
            }

            WHEN( "the new Var is changed") {
                alias_x1 = 10.3;

                THEN( "The original var is also changed") {
                    REQUIRE(x1 == 10.3);
                }
            }
        }

        WHEN( "a new Var of compatible type is created with the same name" ) {
            Var<float> x2("some_double", 1.2);
            Var<int> x3("some_double", 4);

            THEN( "The variables are linked, and converted") {
                REQUIRE(x2 == 3.14f);
                REQUIRE(x3 == 3);
                REQUIRE((void*)&(x2.Get()) != (void*)&(x1.Get()));
                REQUIRE((void*)&(x3.Get()) != (void*)&(x1.Get()));
            }

            WHEN( "The new Var is changed") {
                x3 = 2101;

                THEN( "So is the original") {
                    REQUIRE(x1 == 2101.0);
                }

            }
        }

        WHEN( "a string Var is created with the same name") {
            Var<std::string> xs("some_double", "test");

            REQUIRE( xs.Get() == "3.14" );

            WHEN( "it's changed to a compatible value") {
                xs = "4.14";

                THEN( "the original is updated") {
                    REQUIRE( x1 == 4.14 );
                }
            }

            WHEN( "it's changed to an incompatible value") {
                // This should really throw
                REQUIRE_THROWS_AS(xs = "chortle", BadInputException);

                THEN( "the original remains unchanged") {
                    REQUIRE( x1 == 3.14);
                }
            }
        }

        WHEN( "a new Var of incompatible type is created with the same name") {
            // TODO: Should probably throw here instead
            Var<CustomType> x4("some_double", {1.0,2});

            THEN( "access results in a serialization exception") {
                REQUIRE_THROWS_AS(x4.Get(), BadInputException);
            }
        }

    }
}

SCENARIO( "String Var types" ) {
    // Pangolin Var's have enduring lifetime and we must reset state
    VarState::I().Clear();

    GIVEN( "A string Pangolin Var") {
        Var<std::string> x1("some_string", "0.7");

//        REQUIRE( x1 == "0.7" ); // compile error?
        REQUIRE( x1.Get() == "0.7");
        REQUIRE( x1.Meta().generic == false );

        WHEN( "A specialized Var is instantiated") {
            Var<double> x2("some_string", 0.2);

            THEN( "It takes the original value if compatible'") {
                REQUIRE(x2 == 0.7);
            }

            WHEN( "The specialized Var is updated" ) {
                x2 = 104.89;

                THEN( "The original Var is also updated") {
                    REQUIRE( x1.Get() == "104.89" );
                }
            }

            WHEN( "The original Var is updated" ) {
                x1 = "55.0";

                THEN( "The new var is also updated") {
                    REQUIRE( x2 == 55.0);
                }
            }

            WHEN( "the original var is updated and not compatible") {
                x1 = "sheeple";

                REQUIRE( x1.Get() == std::string("sheeple") );

                THEN( "Accessing the specialized new var causes an exception") {
                    REQUIRE_THROWS_AS(x2.Ref()->Get(), BadInputException);
                    // TODO: Var.Get() current catches and ignores this exception!?
                    REQUIRE_THROWS_AS(x2.Get(), BadInputException);
                }
            }
        }
    }
}

SCENARIO( "Custom Var Types" )
{
    // Pangolin Var's have enduring lifetime and we must reset state
    VarState::I().Clear();

    GIVEN("A Var with custom type with stream operators") {
        Var<CustomType> x1("x1", {1.2f,3});

        THEN("We can access and set it") {
            REQUIRE(x1.Get().a == 1.2f);
            REQUIRE(x1.Get().b == 3);

            x1 = CustomType{4.3f, 90};
            REQUIRE(x1.Get().a == 4.3f);
            REQUIRE(x1.Get().b == 90);
        }

        THEN("We can access it as a string type") {
            Var<std::string> str_x1("x1");
            REQUIRE(str_x1.Get() == "1.2,3");

            str_x1 = "5.6,100";
            REQUIRE(x1.Get().a == 5.6f);
            REQUIRE(x1.Get().b == 100);
        }
    }

    GIVEN("A Var with custom type without stream operators") {
        Var<CustomTypeNoStream> x2("x2", {7.7f, 40});

        THEN("We can access and set it") {
            REQUIRE(x2.Get().a == 7.7f);
            REQUIRE(x2.Get().b == 40);

            x2 = CustomTypeNoStream{4.4f, 91};
            REQUIRE(x2.Get().a == 4.4f);
            REQUIRE(x2.Get().b == 91);
        }

        THEN("Can be accessed through an aliasing Var") {
            Var<CustomTypeNoStream> alias_x2("x2");

            REQUIRE(alias_x2.Get().a == 7.7f);
            REQUIRE(alias_x2.Get().b == 40);

            alias_x2 = CustomTypeNoStream{4.4f, 91};
            REQUIRE(x2.Get().a == 4.4f);
            REQUIRE(x2.Get().b == 91);
        }

        THEN("Generic use through string accessors throws") {
            Var<std::string> str_x2("x2");

            REQUIRE_THROWS(str_x2.Get());
            REQUIRE_THROWS(str_x2 = "1.2,3");
        }
    }
}

SCENARIO("Attaching Vars")
{
    // Pangolin Var's have enduring lifetime and we must reset state
    VarState::I().Clear();

    GIVEN( "An attached Pangolin Var") {
        double x1 = 0.5;
        Var<double>::Attach("x1", x1);

        THEN("The var is accessible through standard vars") {
            REQUIRE( Var<double>("x1").Get() == 0.5 );
            REQUIRE( Var<std::string>("x1").Get() == "0.5");
        }

        WHEN("The attached var is changed") {
            x1 = 10.0;
            REQUIRE(Var<double>("x1").Get() == x1 );
        }

        WHEN("The pangolin var is set") {
            Var<double>("x1") = 2.6;
            REQUIRE(x1 == 2.6);

            Var<std::string>("x1") = "7.8";
            REQUIRE(x1 == 7.8);
        }

        WHEN("Another var is attached with the same name") {
            int x1_int;
            REQUIRE_THROWS_AS(Var<int>::Attach("x1", x1_int), std::runtime_error);
        }

    }

    GIVEN( "An ordinary Pangolin Var") {
        Var<double> x1("x1", 0.5);

        THEN( "Trying to attach a Var with the same name throws") {
            double x1_attach = 1.0;
            REQUIRE_THROWS_AS(Var<double>::Attach("x1",x1_attach) = 2.0, std::runtime_error);
        }
    }
}

SCENARIO("Detaching Vars")
{
    // Pangolin Var's have enduring lifetime and we must reset state
    VarState::I().Clear();

    GIVEN( "An attached Pangolin Var") {
        double x1 = 0.5;
        Var<double>::Attach("x1", x1);

        REQUIRE(VarState::I().Exists("x1"));

        WHEN("An alias is created")
        {
            Var<double> alias_x1("x1");
            REQUIRE(alias_x1.Get() == x1);

            THEN("Changes are synchronized") {
                x1=0.7;
                REQUIRE(alias_x1 == 0.7);

                alias_x1 = 0.9;
                REQUIRE(x1 == 0.9);
            }

            WHEN("Var is detached") {
                DetachVar(x1);

                REQUIRE(!VarState::I().Exists("x1"));

                THEN("Changes are still synchronized since alias still in scope") {
                    x1=0.7;
                    REQUIRE(alias_x1 == 0.7);

                    alias_x1 = 0.9;
                    REQUIRE(x1 == 0.9);
                }

                THEN("Var with same name created is independent") {
                    Var<double> new_x1("x1");
                    REQUIRE(VarState::I().Exists("x1"));

                    new_x1 = 1.2;
                    REQUIRE(x1 == 0.5);

                    x1 = 2.3;
                    REQUIRE(new_x1 == 1.2);

                    WHEN("New Var for x1 created and changed") {
                        Var<double> alias_new_x1("x1");
                        REQUIRE(alias_new_x1 == 1.2);

                        alias_new_x1 = 3.7;
                        REQUIRE(new_x1 == 3.7);
                        REQUIRE(x1 == 2.3);
                    }
                }
            }
        }
    }
}

SCENARIO("Var Events")
{
    // Pangolin Var's have enduring lifetime and we must reset state
    VarState::I().Clear();

    Var<double> x1("x1");
    Var<std::string> x2("x2");

    WHEN("Registering for callbaks without historically added vars") {
        std::vector<std::string> names;

        sigslot::scoped_connection conn = VarState::I().RegisterForVarEvents([&names](const VarState::Event& e){
            REQUIRE(e.action == VarState::Event::Action::Added);
            names.push_back(e.var->Meta().full_name);
        }, false);

        THEN("We dont get them") {
            REQUIRE(names.size() == 0);
        }
    }

    WHEN("Registering for historic events") {
        std::vector<std::string> added;
        std::vector<std::string> removed;
        sigslot::scoped_connection conn1 = VarState::I().RegisterForVarEvents([&added,&removed](const VarState::Event& e){
            auto& vec = (e.action == VarState::Event::Action::Added) ? added : removed;
            vec.push_back(e.var->Meta().full_name);
        }, true);

        THEN("We get them") {
            REQUIRE(added.size() == 2);
            REQUIRE(added[0] == "x1");
            REQUIRE(added[1] == "x2");
        }

        WHEN("We remove a var, we get an event") {
            DetachVarByName("x2");
            REQUIRE(removed.size() == 1);
            REQUIRE(removed[0] == "x2");
        }

        WHEN("We add another listener") {
            sigslot::scoped_connection conn2 = VarState::I().RegisterForVarEvents([&added,&removed](const VarState::Event& e){
                auto& vec = (e.action == VarState::Event::Action::Added) ? added : removed;
                vec.push_back(e.var->Meta().full_name + "_");
            }, false);

            THEN("They both get called!")
            {
                double x3 = 99.0;
                AttachVar("x3", x3);

                REQUIRE(added.size() == 4);
                REQUIRE(added[2] == "x3");
                REQUIRE(added[3] == "x3_");

                DetachVar(x3);
                REQUIRE(removed.size() == 2);
                REQUIRE(removed[0] == "x3");
                REQUIRE(removed[1] == "x3_");
            }
        }
    }
}
