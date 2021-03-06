// Copyright 2011 Google Inc. All Rights Reserved.

package com.googlecode.page_speed.autotester.chrome;

import org.json.simple.JSONObject;

/**
 * A listener that receives notification messages from a Chrome tab (i.e. messages that aren't in
 * direct response to a method call).
 *
 * @author mdsteele@google.com (Matthew D. Steele)
 */
public interface NotificationListener {

  /**
   * Receive a notification message from Chrome.
   *
   * @param method the method name of the notification
   * @param params the params object of the notification
   */
  void handleNotification(String method, JSONObject params);

  /**
   * Called if something goes wrong with the connection to the Chrome tab, such as Chrome sending
   * back malformed JSON or closing the socket unexpectedly.
   *
   * @param message an error message indicating what went wrong
   */
  void onConnectionError(String message);

}
