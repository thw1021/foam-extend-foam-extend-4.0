    Info<< "Reading field p\n" << endl;
    volScalarField pd
    (
        IOobject
        (
            "pd",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    Info<< "Reading field alpha1\n" << endl;
    volScalarField alpha1
    (
        IOobject
        (
            "alpha1",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );


    Info<< "Reading field alpha2\n" << endl;
    volScalarField alpha2
    (
        IOobject
        (
            "alpha2",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );


    Info<< "Reading field alpha3\n" << endl;
    volScalarField alpha3
    (
        IOobject
        (
            "alpha3",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    alpha3 == 1.0 - alpha1 - alpha2;


    Info<< "Reading field U\n" << endl;
    volVectorField U
    (
        IOobject
        (
            "U",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

#   include "createPhi.H"

    threePhaseMixture threePhaseProperties(U, phi);

    const dimensionedScalar& rho1 = threePhaseProperties.rho1();
    const dimensionedScalar& rho2 = threePhaseProperties.rho2();
    const dimensionedScalar& rho3 = threePhaseProperties.rho3();

    dimensionedScalar D23(threePhaseProperties.lookup("D23"));

    // Need to store rho for ddt(rho, U)
    volScalarField rho
    (
        IOobject
        (
            "rho",
            runTime.timeName(),
            mesh,
            IOobject::READ_IF_PRESENT
        ),
        alpha1*rho1 + alpha2*rho2 + alpha3*rho3,
        alpha1.boundaryField().types()
    );
    rho.oldTime();


    // Mass flux
    // Initialisation does not matter because rhoPhi is reset after the
    // alpha solution before it is used in the U equation.
    surfaceScalarField rhoPhi
    (
        IOobject
        (
            "rho*phi",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        rho1*phi
    );

    Info<< "Calculating field g.h\n" << endl;
    volScalarField gh("gh", g & mesh.C());
    surfaceScalarField ghf("gh", g & mesh.Cf());


    volScalarField p
    (
        IOobject
        (
            "p",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        pd + rho*gh
    );

    label pdRefCell = 0;
    scalar pdRefValue = 0.0;
    setRefCell(pd, pimple.dict(), pdRefCell, pdRefValue);
    mesh.schemesDict().setFluxRequired(pd.name());

    scalar pRefValue = 0.0;

    if (pd.needReference())
    {
        pRefValue = readScalar(pimple.dict().lookup("pRefValue"));

        p += dimensionedScalar
        (
            "p",
            p.dimensions(),
            pRefValue - getRefCellValue(p, pdRefCell)
        );
    }

    // Construct interface from alpha distribution
    threePhaseInterfaceProperties interface(threePhaseProperties);


    // Construct incompressible turbulence model
    autoPtr<incompressible::turbulenceModel> turbulence
    (
        incompressible::turbulenceModel::New(U, phi, threePhaseProperties)
    );

    mesh.schemesDict().setFluxRequired(alpha2.name());
