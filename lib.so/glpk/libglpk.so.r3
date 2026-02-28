//
// libglpk.so.r3
// loader for the library solvers gnu linear programming kit
//
static(_LIB_NAME,fileaddr,_HOME_,_LIBD_);

if (exist(_LIB_NAME))
{
  EOF
}

_LIB_NAME = "glpk";

_HOME_ = getenv("HOME");
if(getenv("CPU") == "i686")
{
  _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_libglpk.so";
}
else
{
  _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_lib64glpk.so";
}

fileaddr = _HOME_ + _LIBD_ ;

if (!exist(_glpk_read))
{
  _glpk_read = dlopen(fileaddr, "ent_glpk_read_file");
}

if (!exist(_glpk_write))
{
  _glpk_write = dlopen(fileaddr, "ent_glpk_write_file");
}

if (!exist(_glpk_solve))
{
  _glpk_solve = dlopen(fileaddr, "ent_glpk_solve_lp");
}

/*
 * rlabplus class for linear programming
 *
 *      lp.r3
 *
 * copyright 2017, M. Kostrun
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
lp = classdef(arg1,arg2)
{
  // static = class-member private declaration:
  //    dynamic memory storage that follows the class-member
  static(_objective, _offset, _opt_direction, _problem, _constraints, _bounds_row, _bounds_col);
  static(_nrow, _ncol, _col_int, _col_bin, _solution);

  if (class(arg1)=="string")
  {
    if (isfile(arg1))
    {
      _dummy = _glpk_read (arg1,arg2);
      _objective = _dummy.objective;
      if (exist(_dummy.c0))
      {_offset = _dummy.c0;}
      _opt_direction = _dummy.opt_direction;
      _problem = _dummy.problem;
      _constraints = _dummy.constraints;
      _bounds_row = _dummy.bounds_row;
      _bounds_col = _dummy.bounds_col;
      if (exist(_dummy.col_int))
      { _col_int = _dummy.col_int; }
      if (exist(_dummy.col_bin))
      { _col_bin = _dummy.col_bin; }
      _nrow = _constraints.nr;
      _ncol = _constraints.nc;
    }
  }

  if(!exist(_nrow))
  {
    _nrow = 0;
    _ncol = 0;
    _objective = [];
    _offset = 0;
    _direction = "n/a";
    _problem = "n/a";
    _constraints = [];
    _bounds_row = [];
    _bounds_col = [];
    _col_int = [];
    _col_bin = [];
  }

  // private declaration:
  //  class-members global variables go in here.
  //  if omitted, new variables default to this
  public (_description);
  _description = "linear programming problem";

  // public declaration:
  //    after calling the class constructor these are available as
  //    methods to access and modify class-member static variables
  public (nrow,ncol,objective,offset,opt_direction,problem,constraints,bounds_row,bounds_col,solve,write);
  nrow = function(x)
  {
    if (exist(x))
    { _nrow = x;}
    return _nrow;
  };
  ncol = function(x)
  {
    if (exist(x))
    { _ncol = x;}
    return _ncol;
  };
  objective = function(x)
  {
    if (exist(x))
    {
      _objective = x;
      if (_ncol != length(_objective))
      { _ncol = length(_objective); }
      if (_ncol != length(_offset))
      { _offset = zeros(1,length(_objective)); }
      if (_bounds_col.nr != _ncol)
      { _bounds_col = []; }
    }
    return _objective;
  };
  offset = function(x)
  {
    if (exist(x))
    {
      _offset = x;
      if (_ncol != length(_offset))
      { _ncol = length(_offset); }
      if (_ncol != length(_offset))
      { _objective = zeros(1,length(_offset)); }
      if (_bounds_col.nr != _ncol)
      { _bounds_col = []; }
    }
    return _offset;
  };
  opt_direction = function(x)
  {
    if (exist(x))
    {
      if (tolower(x)=="min" || tolower(x)=="max")
      { _opt_direction= tolower(x); }
    }
    return _opt_direction;
  };
  problem = function(x)
  {
    if (exist(x))
    {
      if (tolower(x)=="lp" || tolower(x)=="mip")
      { _problem = tolower(x); }
    }
    return _problem;
  };
  constraints = function(x)
  {
    if (exist(x))
    {
      _constraints = x;
      if (_nrow != _constraints.nr || _ncol != _constraints.nc)
      {
        _nrow = _constraints.nr;
        _ncol = _constraints.nr;
      }
    }
    return _constraints;
  };
  bounds_row = function(x)
  {
    if (exist(x))
    {
      _bounds_row = x;
      if (_nrow != _bounds_row.nr)
      { _nrow = _bounds_row.nr; }
      if (_nrow != _constraints.nr)
      { _constraints = []; }
    }
    return _bounds_row;
  };
  bounds_col = function(x)
  {
    if (exist(x))
    {
      _bounds_col = x;
      if (_ncol != _bounds_col.nr)
      { _ncol = _bounds_col.nr; }
      if (_ncol != _constraints.nc)
      { _constraints = []; }
    }
    return _bounds_row;
  };
  solve = function(opts)
  {
    if(!exist(opts))
    { opts = <<>>; }
    if (!exist(opts.method))
    { opts.method = "primal"; }
    solution = _glpk_solve(<< ...
          bounds_col = _bounds_col; ...
          bounds_row = _bounds_row; ...
          constraints = _constraints; ...
          objective  = _objective; ...
          opt_direction = _opt_direction; ...
          problem = _problem>>, opts);
    return solution;
  };
  write = function(arg1,arg2)
  {
    if (isstring(arg1))
    {
      _glpk_write(<< ...
          bounds_col = _bounds_col; ...
          bounds_row = _bounds_row; ...
          constraints = _constraints; ...
          objective  = _objective; ...
          opt_direction = _opt_direction; ...
          problem = _problem>>, arg1, arg2);
      return 0;
    }
    return 1;
  };
};


clear(fileaddr,_HOME_,_LIBD_);






