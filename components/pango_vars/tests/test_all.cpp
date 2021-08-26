#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <pangolin/var/var.h>

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
    return is >> x.a >> x.b;
}


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
