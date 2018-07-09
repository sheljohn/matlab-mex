
//==================================================
// @title        J.H. Mex Library
// @author       Jonathan Hadida
// @contact      Jhadida87 [at] gmail
//==================================================

#include "main.h"

// ------------------------------------------------------------------------

namespace jhm {

    void cout_redirect( bool status )
    {
        static std::unique_ptr< coutRedirection<mexPrintf_ostream> > r;

        if ( status && !r )
            r.reset( new coutRedirection<mexPrintf_ostream>() );

        if ( !status )
            r.reset();
    }
    
    // ----------  =====  ----------
    
    int set_field( mxArray *mxs, index_t index, const char *field, mxArray *value )
    {
        JHM_ASSERT( mxs, "Null pointer." );
        JHM_ASSERT( mxIsStruct(mxs), "Input is not a struct." );

        // try to find the corresponding field
        int fnum = mxGetFieldNumber( mxs, field );

        // the field exists, check if there is something there
        if ( fnum >= 0 )
        {
            mxArray *fval = mxGetFieldByNumber( mxs, index, fnum );

            // there is something, so delete it first
            if ( fval ) mxDestroyArray( fval );
        }
        else // the field doesn't exist, so create it
        {
            fnum = mxAddField( mxs, field );
        }

        // set the value now
        mxSetField( mxs, index, field, value );

        return fnum;
    }

    int set_cell( mxArray *mxc, index_t index, mxArray *value )
    {
        JHM_ASSERT( mxc, "Null pointer." );
        JHM_ASSERT( mxIsStruct(mxc), "Input is not a cell." );

        mxSetCell( mxc, index, value );
        return 0; // mxSetCell doesn't return a status...
    }

    int set_variable( MATFile *mtf, const char *name, mxArray *value )
    {
        JHM_ASSERT( mtf, "Null pointer." );
        return matPutVariable( mtf, name, value );
    }
    
    // ----------  =====  ----------
    
    std::string get_string( const mxArray *ms ) 
    {
        JHM_ASSERT( ms, "Null pointer." );
        JHM_ASSERT( mxIsChar(ms), "Input is not a string." );

        std::string val;
        val.resize( mxGetNumberOfElements(ms) );
        mxGetString( ms, &val[0], val.size()+1 );
        return val;
    }
    
    // ----------  =====  ----------
    
    void AbstractMapping::clear() 
    {
        m_fields.clear();
        m_fmap.clear();
    }

    bool AbstractMapping::has_any( const inilst<const char*>& names ) const
    {
        for ( auto& name: names )
                if ( has_field(name) )
                    return true;

            return false;
    }

    bool AbstractMapping::has_fields ( const inilst<const char*>& names ) const
    {
        for ( auto& name: names ) 
            if ( !has_field(name) ) 
            {
                println( "Field '%s' doesn't exist.", name );
                return false;
            }
        return true;
    }
    
    // ----------  =====  ----------
    
    void MAT::clear() 
    {
        if (mfile) matClose(mfile);

        mfile = nullptr;
        AbstractMapping::clear();
    }

    bool MAT::open( const char *name )
    {
        clear();
        JHM_ASSERT( name, "Null filename." );

        MATFile *mf = matOpen( name, "r" );
        JHM_ASSERT( mf, "Error opening file: %s", name );

        int nf = 0;
        const char **fnames = (const char**) matGetDir( mf, &nf );
        JHM_WREJECT( nf == 0, "Empty file." );

        mfile = mf;
        this->m_fields.resize(nf);

        for ( int f = 0; f < nf; ++f )
        {
            this->m_fields[f] = fnames[f];
            this->m_fmap[ this->m_fields[f] ] = matGetVariable( mf, fnames[f] );
        }

        return true;
    }
    
    // ----------  =====  ----------
    
    void Cell::wrap( const mxArray *ms ) 
    {
        JHM_ASSERT( ms, "Null pointer." );
        JHM_ASSERT( mxIsCell(ms), "Input is not a cell." );

        int nc = mxGetNumberOfElements(ms);
        JHM_WREJECT( nc == 0, "Empty cell." );

        mcell = ms;
    }

    // ----------  =====  ----------
    
    void Struct::clear()
    {
        mstruct = nullptr;
        AbstractMapping::clear();
    }

    bool Struct::wrap( const mxArray* ms, index_t index )
    {
        clear();
        JHM_ASSERT( ms, "Null pointer." );
        JHM_ASSERT( mxIsStruct(ms), "Input is not a structure." );

        const index_t nf = mxGetNumberOfFields(ms);
        JHM_WREJECT( nf == 0, "Empty struct." );

        mstruct = ms;
        this->m_fields.resize(nf);

        for ( index_t f = 0; f < nf; ++f )
        {
            this->m_fields[f] = mxGetFieldNameByNumber(ms,f);
            this->m_fmap[ this->m_fields[f] ] = mxGetFieldByNumber(ms,index,f);
        }

        return true;
    }

}