/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/*
**
** Lighting and coloring code.
**
*/
#include "gc_gl_context.h"
#include "gc_gl_debug.h"

extern GLvoid __glConvertResult(__GLcontext *gc, GLint fromType, const GLvoid *rawdata,
                              GLint toType, GLvoid *result, GLint size);


__GL_INLINE GLvoid
__glLightfv(__GLcontext *gc, GLuint light, GLenum pname, GLfloat *pv)
{
    __GLlightSourceState *src;
    __GLcoord dir;
    /* light unsigned, no need for negative check*/
    if (light >= gc->constants.numberOfLights) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    src = &gc->state.light.source[light];
    switch (pname) {
      case GL_AMBIENT:
          src->ambient.r = pv[0];
          src->ambient.g = pv[1];
          src->ambient.b = pv[2];
          src->ambient.a = pv[3];
          __GL_SET_LIGHT_SRC_BIT(gc, light, __GL_LIGHT_AMBIENT_BIT);
          break;

      case GL_DIFFUSE:
          src->diffuse.r = pv[0];
          src->diffuse.g = pv[1];
          src->diffuse.b = pv[2];
          src->diffuse.a = pv[3];
          __GL_SET_LIGHT_SRC_BIT(gc, light, __GL_LIGHT_DIFFUSE_BIT);
          break;

      case GL_SPECULAR:
          src->specular.r = pv[0];
          src->specular.g = pv[1];
          src->specular.b = pv[2];
          src->specular.a = pv[3];
          __GL_SET_LIGHT_SRC_BIT(gc, light, __GL_LIGHT_SPECULAR_BIT);
          break;

      case GL_POSITION:
          src->position.x = pv[0];
          src->position.y = pv[1];
          src->position.z = pv[2];
          src->position.w = pv[3];

          /* Transform light position into eye space.
          */
          if ((src->position.x != 0.0) ||
              (src->position.y != 0.0) ||
              (src->position.z != 0.0) ||
              (src->position.w != 0.0)) {

                  if (src->position.w == 0.0) {
                      /* The light is treated as a directional source */
                      dir.x = src->position.x;
                      dir.y = src->position.y;
                      dir.z = src->position.z;
                      dir.w = 0.0;
                      __glTransformVector(gc, &src->positionEye, &dir, gc->transform.modelView, GL_TRUE);
                      src->positionEye.w = 0;
                  }
                  else {
                      __glTransformCoord(&src->positionEye, &src->position,
                          &gc->transform.modelView->matrix);
                  }
          }
          else {
              src->positionEye.x = 0.0;
              src->positionEye.y = 0.0;
              src->positionEye.z = 1.0;
              src->positionEye.w = 0.0;
          }

          __GL_SET_LIGHT_SRC_BIT(gc, light, __GL_LIGHT_POSITION_BIT);
          break;

      case GL_SPOT_DIRECTION:
          dir.x = pv[0];
          dir.y = pv[1];
          dir.z = pv[2];
          dir.w = 1.0;

          /* Transform light direction into eye space.
          */
          __glTransformVector(gc, &src->direction, &dir, gc->transform.modelView, GL_TRUE);
          src->direction.w = 0;

          __GL_SET_LIGHT_SRC_BIT(gc, light, __GL_LIGHT_SPOTDIRECTION_BIT);
          break;

      case GL_SPOT_EXPONENT:
          if ((pv[0] < 0.) || (pv[0] > 128.)) {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          src->spotLightExponent = pv[0];
          __GL_SET_LIGHT_SRC_BIT(gc, light, __GL_LIGHT_SPOTEXPONENT_BIT);
          break;

      case GL_SPOT_CUTOFF:
          if ((pv[0] != 180.) && ((pv[0] < 0.) || (pv[0] > 90.))) {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          src->spotLightCutOffAngle = pv[0];
          __GL_SET_LIGHT_SRC_BIT(gc, light, __GL_LIGHT_SPOTCUTOFF_BIT);
          break;

      case GL_CONSTANT_ATTENUATION:
          if (pv[0] < __glZero) {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          src->constantAttenuation = pv[0];
          __GL_SET_LIGHT_SRC_BIT(gc, light, __GL_LIGHT_CONSTANTATT_BIT);
          break;

      case GL_LINEAR_ATTENUATION:
          if (pv[0] < __glZero) {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          src->linearAttenuation = pv[0];
          __GL_SET_LIGHT_SRC_BIT(gc, light, __GL_LIGHT_LINEARATT_BIT);
          break;

      case GL_QUADRATIC_ATTENUATION:
          if (pv[0] < __glZero) {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          src->quadraticAttenuation = pv[0];
          __GL_SET_LIGHT_SRC_BIT(gc, light, __GL_LIGHT_QUADRATICATT_BIT);
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_Lightfv(GLenum light, GLenum pname, const GLfloat *pv)
{
    GLuint lightIndex = ((GLuint)light - GL_LIGHT0);
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Lightfv", DT_GLenum, light, DT_GLenum, pname, DT_GLfloat_ptr, pv, DT_GLnull);
#endif

    __glLightfv(gc, lightIndex, pname, (GLfloat*)pv);
}

GLvoid APIENTRY __glim_Lightf(GLenum light, GLenum pname, GLfloat f)
{
    GLfloat tmpf[4];
    GLuint lightIndex = (light - GL_LIGHT0);
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Lightf", DT_GLenum, light, DT_GLenum, pname, DT_GLfloat, f, DT_GLnull);
#endif

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_SPOT_EXPONENT:
      case GL_SPOT_CUTOFF:
      case GL_CONSTANT_ATTENUATION:
      case GL_LINEAR_ATTENUATION:
      case GL_QUADRATIC_ATTENUATION:
          tmpf[0] = f;
          __glLightfv(gc, lightIndex, pname, (GLfloat*)tmpf);
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          break;
    }
}

GLvoid APIENTRY __glim_Lightiv(GLenum light, GLenum pname, const GLint *pv)
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    GLuint lightIndex = (light - GL_LIGHT0);
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Lightiv", DT_GLenum, light, DT_GLenum, pname, DT_GLint_ptr, pv, DT_GLnull);
#endif

    switch (pname) {
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
          tmpf[0] = __GL_I_TO_FLOAT(pv[0]);
          tmpf[1] = __GL_I_TO_FLOAT(pv[1]);
          tmpf[2] = __GL_I_TO_FLOAT(pv[2]);
          tmpf[3] = __GL_I_TO_FLOAT(pv[3]);
          break;
      case GL_POSITION:
      case GL_SPOT_DIRECTION:
          tmpf[0] = (GLfloat)(pv[0]);
          tmpf[1] = (GLfloat)(pv[1]);
          tmpf[2] = (GLfloat)(pv[2]);
          tmpf[3] = (GLfloat)(pv[3]);
          break;
      case GL_SPOT_EXPONENT:
      case GL_SPOT_CUTOFF:
      case GL_CONSTANT_ATTENUATION:
      case GL_LINEAR_ATTENUATION:
      case GL_QUADRATIC_ATTENUATION:
          tmpf[0] = (GLfloat)(pv[0]);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
    __glLightfv(gc, lightIndex, pname, tmpf);
}

GLvoid APIENTRY __glim_Lighti(GLenum light, GLenum pname, GLint i)
{
    GLfloat tmpf[4] ;
    GLuint lightIndex = (light - GL_LIGHT0);
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Lighti", DT_GLenum, light, DT_GLenum, pname, DT_GLint, i, DT_GLnull);
#endif

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_SPOT_EXPONENT:
      case GL_SPOT_CUTOFF:
      case GL_CONSTANT_ATTENUATION:
      case GL_LINEAR_ATTENUATION:
      case GL_QUADRATIC_ATTENUATION:
          tmpf[0] = (GLfloat)i;
          __glLightfv(gc, lightIndex, pname, (GLfloat*)tmpf);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}


__GL_INLINE GLvoid
__glLightModelfv(__GLcontext *gc, GLenum pname, GLfloat *pv)
{
    __GLlightModelState *lightmodel = &(gc->state.light.model);
    GLenum param;

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname) {
      case GL_LIGHT_MODEL_AMBIENT:
          lightmodel->ambient.r = pv[0];
          lightmodel->ambient.g = pv[1];
          lightmodel->ambient.b = pv[2];
          lightmodel->ambient.a = pv[3];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_LIGHTMODEL_AMBIENT_BIT);
          break;

      case GL_LIGHT_MODEL_LOCAL_VIEWER:
          lightmodel->localViewer = (pv[0] != 0.0);
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_LIGHTMODEL_LOCALVIEWER_BIT);
          break;

      case GL_LIGHT_MODEL_TWO_SIDE:
          lightmodel->twoSided = (pv[0] != 0.0);
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_LIGHTMODEL_TWOSIDE_BIT);
          break;

      case GL_LIGHT_MODEL_COLOR_CONTROL:
          param = (GLenum)pv[0];
          if (GL_SINGLE_COLOR != param && GL_SEPARATE_SPECULAR_COLOR != param) {
              __glSetError(GL_INVALID_ENUM);
              return;
          }
          lightmodel->colorControl = param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_LIGHTMODEL_COLORCONTROL_BIT);
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_LightModelfv(GLenum pname, const GLfloat *pv)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_LightModelfv", DT_GLenum, pname, DT_GLfloat_ptr, pv, DT_GLnull);
#endif

    __glLightModelfv(gc, pname, (GLfloat*)pv);
}

GLvoid APIENTRY __glim_LightModelf(GLenum pname, GLfloat f)
{
    GLfloat tmpf[4] ;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_LightModelf", DT_GLenum, pname, DT_GLfloat, f, DT_GLnull);
#endif

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
      case GL_LIGHT_MODEL_TWO_SIDE:
      case GL_LIGHT_MODEL_COLOR_CONTROL:
          tmpf[0] = f;
          __glLightModelfv(gc, pname, (GLfloat*)tmpf);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_LightModeliv(GLenum pname, const GLint *pv)
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_LightModeliv", DT_GLenum, pname, DT_GLint_ptr, pv, DT_GLnull);
#endif

    switch (pname) {
      case GL_LIGHT_MODEL_AMBIENT:
          tmpf[0] = __GL_I_TO_FLOAT(pv[0]);
          tmpf[1] = __GL_I_TO_FLOAT(pv[1]);
          tmpf[2] = __GL_I_TO_FLOAT(pv[2]);
          tmpf[3] = __GL_I_TO_FLOAT(pv[3]);
          break;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
      case GL_LIGHT_MODEL_TWO_SIDE:
      case GL_LIGHT_MODEL_COLOR_CONTROL:
          tmpf[0] = (GLfloat)pv[0];
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
    __glLightModelfv(gc, pname, (GLfloat*)tmpf);
}

GLvoid APIENTRY __glim_LightModeli(GLenum pname, GLint i)
{
    GLfloat tmpf[4] ;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_LightModeli", DT_GLenum, pname, DT_GLint, i, DT_GLnull);
#endif

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
      case GL_LIGHT_MODEL_TWO_SIDE:
      case GL_LIGHT_MODEL_COLOR_CONTROL:
          tmpf[0] = (GLfloat)i;
          __glLightModelfv(gc, pname, (GLfloat*)tmpf);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_ColorMaterial(GLenum face, GLenum p)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ColorMaterial", DT_GLenum, face, DT_GLenum, p, DT_GLnull);
#endif

    switch (face)
    {
    case GL_FRONT:
    case GL_BACK:
    case GL_FRONT_AND_BACK:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (p)
    {
    case GL_EMISSION:
    case GL_SPECULAR:
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_AMBIENT_AND_DIFFUSE:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->state.light.colorMaterialFace = face;
    gc->state.light.colorMaterialParam = p;

    /* Use the current color to update material state
    * if color material is already enabled
    */
    if (gc->state.enables.lighting.colorMaterial) {
        __glUpdateMaterialfv(gc, face, p, (GLfloat *)&gc->state.current.color);
    }

    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_COLORMATERIAL_BIT);
}

GLvoid APIENTRY __glim_GetLightfv(GLenum light, GLenum pname, GLfloat *result)
{
    GLint index;
    __GLlightSourceState *src;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetLightfv", DT_GLenum, light, DT_GLenum, pname, DT_GLfloat_ptr, result, DT_GLnull);
#endif

    index = light - GL_LIGHT0;
    if ((index < 0) || (index >= (GLint)gc->constants.numberOfLights)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    src = &gc->state.light.source[index];
    switch (pname)
    {
    case GL_AMBIENT:
        result[0] = src->ambient.r;
        result[1] = src->ambient.g;
        result[2] = src->ambient.b;
        result[3] = src->ambient.a;
        break;
    case GL_DIFFUSE:
        result[0] = src->diffuse.r;
        result[1] = src->diffuse.g;
        result[2] = src->diffuse.b;
        result[3] = src->diffuse.a;
        break;
    case GL_SPECULAR:
        result[0] = src->specular.r;
        result[1] = src->specular.g;
        result[2] = src->specular.b;
        result[3] = src->specular.a;
        break;
    case GL_POSITION:
        result[0] = src->positionEye.x;
        result[1] = src->positionEye.y;
        result[2] = src->positionEye.z;
        result[3] = src->positionEye.w;
        break;
    case GL_SPOT_DIRECTION:
        result[0] = src->direction.x;
        result[1] = src->direction.y;
        result[2] = src->direction.z;
        break;
    case GL_SPOT_EXPONENT:
        result[0] = src->spotLightExponent;
        break;
    case GL_SPOT_CUTOFF:
        result[0] = src->spotLightCutOffAngle;
        break;
    case GL_CONSTANT_ATTENUATION:
        result[0] = src->constantAttenuation;
        break;
    case GL_LINEAR_ATTENUATION:
        result[0] = src->linearAttenuation;
        break;
    case GL_QUADRATIC_ATTENUATION:
        result[0] = src->quadraticAttenuation;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetLightiv(GLenum light, GLenum pname, GLint *result)
{
    GLint index;
    __GLlightSourceState *src;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetLightiv", DT_GLenum, light, DT_GLenum, pname, DT_GLint_ptr, result, DT_GLnull);
#endif

    index = light - GL_LIGHT0;
    if ((index < 0) || (index >= (GLint)gc->constants.numberOfLights)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    src = &gc->state.light.source[index];
    switch (pname)
    {
    case GL_AMBIENT:
        result[0] = __GL_FLOAT_TO_I(src->ambient.r);
        result[1] = __GL_FLOAT_TO_I(src->ambient.g);
        result[2] = __GL_FLOAT_TO_I(src->ambient.b);
        result[3] = __GL_FLOAT_TO_I(src->ambient.a);
        break;
    case GL_DIFFUSE:
        result[0] = __GL_FLOAT_TO_I(src->diffuse.r);
        result[1] = __GL_FLOAT_TO_I(src->diffuse.g);
        result[2] = __GL_FLOAT_TO_I(src->diffuse.b);
        result[3] = __GL_FLOAT_TO_I(src->diffuse.a);
        break;
    case GL_SPECULAR:
        result[0] = __GL_FLOAT_TO_I(src->specular.r);
        result[1] = __GL_FLOAT_TO_I(src->specular.g);
        result[2] = __GL_FLOAT_TO_I(src->specular.b);
        result[3] = __GL_FLOAT_TO_I(src->specular.a);
        break;
    case GL_POSITION:
        __glConvertResult(gc, __GL_FLOAT, &src->positionEye.x,
            __GL_INT32, result, 4);
        break;
    case GL_SPOT_DIRECTION:
        __glConvertResult(gc, __GL_FLOAT, &src->direction.x,
            __GL_INT32, result, 3);
        break;
    case GL_SPOT_EXPONENT:
        __glConvertResult(gc, __GL_FLOAT, &src->spotLightExponent,
            __GL_INT32, result, 1);
        break;
    case GL_SPOT_CUTOFF:
        __glConvertResult(gc, __GL_FLOAT, &src->spotLightCutOffAngle,
            __GL_INT32, result, 1);
        break;
    case GL_CONSTANT_ATTENUATION:
        __glConvertResult(gc, __GL_FLOAT, &src->constantAttenuation,
            __GL_INT32, result, 1);
        break;
    case GL_LINEAR_ATTENUATION:
        __glConvertResult(gc, __GL_FLOAT, &src->linearAttenuation,
            __GL_INT32, result, 1);
        break;
    case GL_QUADRATIC_ATTENUATION:
        __glConvertResult(gc, __GL_FLOAT, &src->quadraticAttenuation,
            __GL_INT32, result, 1);
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid __glInitLightState(__GLcontext *gc)
{
    __GLlightState *ls = &gc->state.light;
    __GLlightSourceState *src;
    GLint i;

    ls->model.ambient.r = 0.2f;
    ls->model.ambient.g = 0.2f;
    ls->model.ambient.b = 0.2f;
    ls->model.ambient.a = 1.0f;
    ls->model.colorControl = GL_SINGLE_COLOR;

    ls->shadingModel = GL_SMOOTH;
    ls->colorMaterialFace = GL_FRONT_AND_BACK;
    ls->colorMaterialParam = GL_AMBIENT_AND_DIFFUSE;

    ls->front.ambient.r = 0.2f;
    ls->front.ambient.g = 0.2f;
    ls->front.ambient.b = 0.2f;
    ls->front.ambient.a = 1.0f;

    ls->front.diffuse.r = 0.8f;
    ls->front.diffuse.g = 0.8f;
    ls->front.diffuse.b = 0.8f;
    ls->front.diffuse.a = 1.0f;

    ls->front.specular.r = 0.0f;
    ls->front.specular.g = 0.0f;
    ls->front.specular.b = 0.0f;
    ls->front.specular.a = 1.0f;

    ls->front.emissive.r = 0.0f;
    ls->front.emissive.g = 0.0f;
    ls->front.emissive.b = 0.0f;
    ls->front.emissive.a = 1.0f;

    ls->front.cmapa = 0;
    ls->front.cmaps = 1;
    ls->front.cmapd = 1;

    ls->back = ls->front;

    ls->clampVertexColor = GL_TRUE;

    /* Initialize the individual lights */
    for (i = 0; i < (GLint)gc->constants.numberOfLights; i++, src++) {
        gc->state.enables.lighting.light[i] = GL_FALSE;

        src = &ls->source[i];

        src->ambient.r = src->ambient.g = src->ambient.b = 0.0;
        src->ambient.a = 1.0;
        if (i == 0) {
            src->diffuse.r = src->diffuse.g = src->diffuse.b = 1.0;
            src->diffuse.a = 1.0;
        } else {
            src->diffuse.r = src->diffuse.g = src->diffuse.b = 0.0;
            src->diffuse.a = 1.0;
        }
        src->specular = src->diffuse;
        src->position.z = 1.0f;
        src->positionEye.z = 1.0f;
        src->direction.z = -1.0f;
        src->spotLightCutOffAngle = 180.0f;
        src->constantAttenuation = 1.0f;
        src->linearAttenuation = 0.0f;
        src->quadraticAttenuation = 0.0f;
    }
}


