    Info<< "Reading field p\n" << endl;
    volScalarField p
    (
        IOobject
        (
            "p",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    Info<< "Reading field alpha\n" << endl;
    volScalarField alpha
    (
        IOobject
        (
            "alpha",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

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


    Info<< "Reading transportProperties\n" << endl;

    IOdictionary transportProperties
    (
        IOobject
        (
            "transportProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );


    dimensionedScalar rhoc
    (
        transportProperties.lookup("rhoc")
    );

    dimensionedScalar rhod
    (
        transportProperties.lookup("rhod")
    );

    dimensionedScalar muc
    (
        transportProperties.lookup("muc")
    );

    dimensionedScalar plasticViscosityCoeff
    (
        transportProperties.lookup("plasticViscosityCoeff")
    );

    dimensionedScalar plasticViscosityExponent
    (
        transportProperties.lookup("plasticViscosityExponent")
    );

    dimensionedScalar yieldStressCoeff
    (
        transportProperties.lookup("yieldStressCoeff")
    );

    dimensionedScalar yieldStressExponent
    (
        transportProperties.lookup("yieldStressExponent")
    );

    dimensionedScalar yieldStressOffset
    (
        transportProperties.lookup("yieldStressOffset")
    );

    Switch BinghamPlastic
    (
        transportProperties.lookup("BinghamPlastic")
    );

    volScalarField rho
    (
        IOobject
        (
            "rho",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (scalar(1) - alpha)*rhoc + alpha*rhod
    );

    volScalarField Alpha
    (
        IOobject
        (
            "Alpha",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        alpha*rhod/rho,
        alpha.boundaryField().types()
    );

#   include "compressibleCreatePhi.H"


    label pRefCell = 0;
    scalar pRefValue = 0.0;
    setRefCell(p, pimple.dict(), pRefCell, pRefValue);
    mesh.schemesDict().setFluxRequired(p.name());

    Info<< "Calculating field mul\n" << endl;
    volScalarField mul
    (
        IOobject
        (
            "mul",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        muc +
        plasticViscosity
        (
            plasticViscosityCoeff,
            plasticViscosityExponent,
            Alpha
        )
    );


    Info<< "Initialising field Vdj\n" << endl;
    volVectorField Vdj
    (
        IOobject
        (
            "Vdj",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedVector("0.0", U.dimensions(), vector::zero),
        U.boundaryField().types()
    );


    Info<< "Selecting Drift-Flux model " << endl;

    word VdjModel
    (
        transportProperties.lookup("VdjModel")
    );

    Info<< tab << VdjModel << " selected\n" << endl;

    const dictionary& VdjModelCoeffs
    (
        transportProperties.subDict(VdjModel + "Coeffs")
    );

    dimensionedVector V0
    (
        VdjModelCoeffs.lookup("V0")
    );

    dimensionedScalar a
    (
        VdjModelCoeffs.lookup("a")
    );

    dimensionedScalar a1
    (
        VdjModelCoeffs.lookup("a1")
    );

    dimensionedScalar alphaMin
    (
        VdjModelCoeffs.lookup("alphaMin")
    );



    IOdictionary RASProperties
    (
        IOobject
        (
            "RASProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );


    Switch turbulence
    (
        RASProperties.lookup("turbulence")
    );

    const dictionary& kEpsilonCoeffs
    (
        RASProperties.subDict("kEpsilonCoeffs")
    );


    scalar Cmu
    (
        readScalar(kEpsilonCoeffs.lookup("Cmu"))
    );

    scalar C1
    (
        readScalar(kEpsilonCoeffs.lookup("C1"))
    );

    scalar C2
    (
        readScalar(kEpsilonCoeffs.lookup("C2"))
    );

    scalar C3
    (
        readScalar(kEpsilonCoeffs.lookup("C3"))
    );

    scalar alphak
    (
        readScalar(kEpsilonCoeffs.lookup("alphak"))
    );

    scalar alphaEps
    (
        readScalar(kEpsilonCoeffs.lookup("alphaEps"))
    );

    const dictionary& wallFunctionCoeffs
    (
        RASProperties.subDict("wallFunctionCoeffs")
    );

    scalar kappa
    (
        readScalar(wallFunctionCoeffs.lookup("kappa"))
    );

    scalar E
    (
        readScalar(wallFunctionCoeffs.lookup("E"))
    );

    nearWallDist y(mesh);

    Info<< "Reading field k\n" << endl;
    volScalarField k
    (
        IOobject
        (
            "k",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    Info<< "Reading field epsilon\n" << endl;
    volScalarField epsilon
    (
        IOobject
        (
            "epsilon",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    Info<< "Calculating field mut\n" << endl;
    volScalarField mut
    (
        IOobject
        (
            "mut",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        Cmu*rho*sqr(k)/epsilon
    );


    Info<< "Calculating field mu\n" << endl;
    volScalarField mu
    (
        IOobject
        (
            "mu",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mut + mul
    );

    Info<< "Calculating field (g.h)f\n" << endl;
    surfaceScalarField ghf = surfaceScalarField("gh", g & mesh.Cf());
