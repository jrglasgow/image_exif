<?php

/**
 * @file
 *
 * contain the image toolkit necessary callbacks
 */

 /**
 * @see image_gd_settings().
 */
function image_exif_settings() {
  module_load_include('inc', 'system', 'image.gd');
  $form =  image_gd_settings();
  $form['status']['#markup'] .= ' ' . t('EXIF Metadata Retention relies heavily on "GD2 image manipulation toolkit".');
  return $form;
}

/**
 * @see image_gd_settings_validate().
 */
function image_exif_settings_validate($form, &$form_state) {
  module_load_include('inc', 'system', 'image.gd');
  image_gd_settings_validate($form, $form_state);
}

/**
 * @see image_gd_resize().
 */
function image_exif_resize(stdClass $image, $width, $height) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_resize($image, $width, $height);
}

/**
 * @see image_gd_rotate().
 */
function image_exif_rotate(stdClass $image, $degrees, $background = NULL) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_rotate($image, $degrees, $background = NULL);
}

/**
 * @see image_gd_crop().
 */
function image_exif_crop(stdClass $image, $x, $y, $width, $height) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_crop($image, $x, $y, $width, $height);
}

/**
 * @see image_gd_desaturate().
 */
function image_exif_desaturate(stdClass $image) {
  return image_gd_desaturate($image);
}

/**
 * @see image_gd_load().
 */
function image_exif_load(stdClass $image) {
  return image_gd_load($image);
}

/**
 * GD helper to write an image resource to a destination file.
 *
 * @param $image
 *   An image object.
 * @param $destination
 *   A string file URI or path where the image should be saved.
 * @return
 *   TRUE or FALSE, based on success.
 *
 * @see image_save()
 * @see image_gd_save().
 */
function image_exif_save(stdClass $image, $destination) {
  $scheme = file_uri_scheme($destination);
  // Work around lack of stream wrapper support in imagejpeg() and imagepng().
  if ($scheme && file_stream_wrapper_valid_scheme($scheme)) {
    // If destination is not local, save image to temporary local file.
    $local_wrappers = file_get_stream_wrappers(STREAM_WRAPPERS_LOCAL);
    if (!isset($local_wrappers[$scheme])) {
      $permanent_destination = $destination;
      $destination = drupal_tempnam('temporary://', 'gd_');
    }
    // Convert stream wrapper URI to normal path.
    $destination = drupal_realpath($destination);
  }

  $extension = str_replace('jpg', 'jpeg', $image->info['extension']);
  $function = 'image' . $extension;
  if (!function_exists($function)) {
    return FALSE;
  }
  if ($extension == 'jpeg') {
    $success = $function($image->resource, $destination, variable_get('image_jpeg_quality', 75));
    image_exif_save_metadata($image, $destination);
  }
  else {
    // Always save PNG images with full transparency.
    if ($extension == 'png') {
      imagealphablending($image->resource, FALSE);
      imagesavealpha($image->resource, TRUE);
    }
    $success = $function($image->resource, $destination);
  }
  // Move temporary local file to remote destination.
  if (isset($permanent_destination) && $success) {
    return (bool) file_unmanaged_move($destination, $permanent_destination, FILE_EXISTS_REPLACE);
  }
  return $success;
}

/**
 * get the exif data from the original file, allow other modules to alter, save
 * to new derivative file
 */
function image_exif_save_metadata($image, $derivative_path) {
  libraries_load('pjmt');

  if ($wrapper = file_stream_wrapper_get_instance_by_uri($image->source)) {
    $original_path = $wrapper->realpath();
  }
  else {
    return;
  }
  ob_start();


  // read exif data
  $exif = get_EXIF_JPEG($original_path);

  $original_header_data = get_jpeg_header_data($original_path);
  $original_irb = get_Photoshop_IRB($original_header_data);
  $original_iptc = get_Photoshop_IPTC($original_irb);
  $xmp = get_EXIF_JPEG($original_path);

  $out1 = ob_get_contents();
  ob_end_clean();
  // write the same exif data to the derivatives
  $derivative_header_data = get_jpeg_header_data($derivative_path);
  if ($derivative_header_data) {
    $derivative_header_data = put_EXIF_JPEG($exif, $derivative_header_data);
    $derivative_irb = get_Photoshop_IRB($derivative_header_data);
    if (!empty($derivative_irb)) {
      $derivative_irb = put_Photoshop_IPTC($derivative_irb, $original_iptc);
    }
    else {
      $derivative_irb = $original_irb;
    }
    $derivative_header_data = put_Photoshop_IRB($derivative_header_data, $derivative_irb);
    $status = put_jpeg_header_data($derivative_path, $derivative_path, $derivative_header_data);
  }
}

/**
 * @see image_gd_create_tmp().
 */
function image_exif_create_tmp(stdClass $image, $width, $height) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_create_tmp($image, $width, $height);
}

/**
 * @see image_gd_get_info().
 */
function image_exif_get_info(stdClass $image) {
  module_load_include('inc', 'system', 'image.gd');
  $details = image_gd_get_info($image);
  return $details;
}